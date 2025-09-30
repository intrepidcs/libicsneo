#include <iostream>
#include <random>

#include "icsneo/icsneocpp.h"

enum class MessageType {
	ShortEth,
	LongEth,
	CAN,
	CANFD
};

const std::vector<std::string> MessageTypeLabels = {"short Ethernet", "long Ethernet", "CAN", "CAN-FD"};

void onEvent(std::shared_ptr<icsneo::APIEvent> event) {
	std::cout << event->describe() << std::endl;
}

std::vector<std::shared_ptr<icsneo::Frame>> constructRandomFrames(size_t frameCount, MessageType frameType) {
	static constexpr size_t ClassicCANSize = 8;
	static constexpr size_t CANFDSize = 64;
	static constexpr size_t ShortEthSize = 500;
	static constexpr size_t LongEthSize = 1500;

	std::vector<std::shared_ptr<icsneo::Frame>> frames;
	std::random_device randDev;
	std::mt19937 randEngine(randDev());
	std::uniform_int_distribution randByteDist(0,255);
	auto randByteGen = [&]() -> uint8_t {
		return static_cast<uint8_t>(randByteDist(randEngine));
	};

	for(size_t i = 0; i < frameCount; i++) {
		switch(frameType) {
			case MessageType::ShortEth:
				// Short Ethernet
				{
					auto frame = std::make_shared<icsneo::EthernetMessage>();
					frame->network = icsneo::Network::NetID::ETHERNET_01;
					frames.push_back(frame);
					frame->data.resize(ShortEthSize);
					std::generate(frame->data.begin(), frame->data.end(), randByteGen);
				}
				break;
			case MessageType::LongEth:
				// Long Ethernet
				{
					auto frame = std::make_shared<icsneo::EthernetMessage>();
					frame->network = icsneo::Network::NetID::ETHERNET_01;
					frames.push_back(frame);
					frame->data.resize(LongEthSize);
					std::generate(frame->data.begin(), frame->data.end(), randByteGen);
				}
				break;
			case MessageType::CAN:
				// Classic CAN
				{
					auto frame = std::make_shared<icsneo::CANMessage>();
					frame->network = icsneo::Network::NetID::DWCAN_02;
					frames.push_back(frame);
					frame->data.resize(ClassicCANSize);

					std::generate(frame->data.begin(), frame->data.end(), randByteGen);
				}
				break;
			case MessageType::CANFD:
				// CAN FD
				{
					auto frame = std::make_shared<icsneo::CANMessage>();
					frame->network = icsneo::Network::NetID::DWCAN_03;
					frames.push_back(frame);
					frame->data.resize(CANFDSize);
					std::generate(frame->data.begin(), frame->data.end(), randByteGen);
					frame->isCANFD = true;
				}
				break;
		}
	}
	return frames;
}

void resetScriptStatus(std::shared_ptr<icsneo::Device> rxDevice, std::shared_ptr<icsneo::Device> txDevice, bool rxInit, bool txInit) {
	if(rxInit) {
		rxDevice->startScript();
	} else {
		rxDevice->stopScript();
	}
	if(txInit) {
		txDevice->startScript();
	} else {
		txDevice->stopScript();
	}
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		std::cout << "Usage: libicsneocpp-vsa-example <Tx device serial> <Rx device serial>" << std::endl;
		return -1;
	}

	std::string txSerial = argv[1];
	std::string rxSerial = argv[2];

	// register an event callback so we can see all logged events
	icsneo::EventManager::GetInstance().addEventCallback(icsneo::EventCallback(onEvent));

	auto devices = icsneo::FindAllDevices();
	if(devices.empty()) {
		std::cout << "error: no devices found" << std::endl;
		return -1;
	}

	std::shared_ptr<icsneo::Device> txDevice;
	std::shared_ptr<icsneo::Device> rxDevice;
	for(auto&& d : devices) {
		if(txSerial == d->getSerial()) {
			txDevice = d;
		}
		if(rxSerial == d->getSerial()) {
			rxDevice = d;
		}
	}
	if(!txDevice) {
		std::cout << "error: failed to find a device with serial number: " << txSerial << std::endl;
		return -1;
	}
	if(!rxDevice) {
		std::cout << "error: failed to find a device with serial number: " << rxSerial << std::endl;
		return -1;
	}

	std::cout << "info: found " << txDevice->describe() << std::endl;
	std::cout << "info: found " << rxDevice->describe() << std::endl;

	if(!txDevice->open()) {
		std::cout << "error: unable to open device" << std::endl;
	}
	if(!rxDevice->open()) {
		std::cout << "error: unable to open device" << std::endl;
	}

	const auto& rxInitialCoreminiStatus = rxDevice->getScriptStatus()->isCoreminiRunning;
	const auto& txInitialCoreminiStatus = txDevice->getScriptStatus()->isCoreminiRunning;

	rxDevice->stopScript();
	txDevice->stopScript();

	uint64_t origFirstOffset;
	std::shared_ptr<icsneo::VSA> origFirstRecord;
	if(!rxDevice->findFirstVSARecord(origFirstOffset, origFirstRecord)) {
		std::cout << "error: unable to find first VSA record" << std::endl;
		resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);
		return -1;
	}
	std::cout << "info: found first VSA record at " << origFirstOffset << std::endl;

	uint64_t origLastOffset;
	std::shared_ptr<icsneo::VSA> origLastRecord;
	if(!rxDevice->findLastVSARecord(origLastOffset, origLastRecord)) {
		std::cout << "error: unable to find last VSA record" << std::endl;
		resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);
		return -1;
	}
	std::cout << "info: found last VSA record at " << origLastOffset << std::endl;

	uint64_t canFrameCount = 0;
	uint64_t ethFrameCount = 0;
	rxDevice->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> msg) {
		if(msg->type != icsneo::Message::Type::Frame) {
			return;
		}
		const auto frame = std::static_pointer_cast<icsneo::Frame>(msg);
		if(frame->network.getType() == icsneo::Network::Type::CAN) {
			++canFrameCount;
		} else if(frame->network.getType() == icsneo::Network::Type::Ethernet) {
			++ethFrameCount;
		}
	}));
	icsneo::VSAExtractionSettings settings;
	{
		auto& filter = settings.filters.emplace_back();
		filter.readRange.first = origLastRecord->getTimestampICSClock() - std::chrono::seconds(20);
		filter.readRange.second = origLastRecord->getTimestampICSClock() - std::chrono::seconds(10);
	}
	{
		auto& filter = settings.filters.emplace_back();
		filter.readRange.first = origFirstRecord->getTimestampICSClock() + std::chrono::seconds(0);
		filter.readRange.second = origFirstRecord->getTimestampICSClock() + std::chrono::seconds(10);
	}
	std::cout << "info: reading two blocks of VSA, 10s from the start and 10s from the end..." << std::endl;
	if(!rxDevice->readVSA(settings)) {
		std::cout << "error: unable to read VSA" << std::endl;
		resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);
		return -1;
	}
	std::cout << "info: processed " << canFrameCount << " CAN frames and " << ethFrameCount << " Ethernet frames" << std::endl;

	rxDevice->startScript();
	txDevice->goOnline();

	const uint8_t NumFrameTypes = 4;
	const size_t FrameCountPerType = 2500;
	std::vector<std::shared_ptr<icsneo::Frame>> frames;
	for(uint8_t i = 0; i < NumFrameTypes; i++) {
		std::cout << "info: transmitting " << FrameCountPerType << " random " << MessageTypeLabels[i] << " frames" << std::endl;
		auto tempFrames = constructRandomFrames(FrameCountPerType, static_cast<MessageType>(i));
		frames.insert(frames.end(), tempFrames.begin(), tempFrames.end());
		if(!txDevice->transmit(tempFrames)) {
			std::cout << "error: failed to transmit frames" << std::endl;
			resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);
			return -1;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(600));
	}

	size_t currentMessage = 0;
	rxDevice->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> msg) {
		if(msg->type != icsneo::Message::Type::Frame) {
			return;
		}
		auto frame = std::static_pointer_cast<icsneo::Frame>(msg);
		if(frames[currentMessage]->data == frame->data) {
			currentMessage++;
		}
	}));

	// Read from original last frame until end of buffer
	icsneo::VSAExtractionSettings transmitSettings;
	{
		auto& filter = transmitSettings.filters.emplace_back();
		filter.readRange.first = origLastRecord->getTimestampICSClock();
	}

	std::cout << "info: reading transmitted random frames..." << std::endl;
	if(!rxDevice->readVSA(transmitSettings)) {
		std::cout << "error: unable to read VSA" << std::endl;
		resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);
		return -1;
	}

    std::cout << "Waiting for 5 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

	std::cout << "info: " << currentMessage << " transmitted frames found" << std::endl;
	resetScriptStatus(rxDevice, txDevice, rxInitialCoreminiStatus, txInitialCoreminiStatus);

	if(currentMessage != FrameCountPerType * NumFrameTypes) {
		std::cout << "error: unable to find all transmitted frames" << std::endl;
		return -1;
	}

	return 0;
}