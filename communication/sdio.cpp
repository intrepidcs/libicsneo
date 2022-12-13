#include <cstring>

#include "icsneo/communication/sdio.h"
#include "icsneo/platform/sharedsemaphore.h"
#include "icsneo/platform/sharedmemory.h"
#include "icsneo/device/device.h"
#include "icsneo/communication/socket.h"

using namespace icsneo;

void SDIO::Find(std::vector<FoundDevice>& found) {
	auto socket = lockSocket();
	if(!socket.writeTyped(RPC::DEVICE_FINDER_FIND_ALL))
		return;
	uint16_t count;
	if(!socket.readTyped(count))
		return;

	static constexpr auto serialSize = sizeof(FoundDevice::serial);
	std::vector<std::array<char, sizeof(FoundDevice::serial)>> serials(count);
	if(!socket.read(serials.data(), serials.size() * serialSize))
		return;

	for(const auto& serial : serials) {
		auto& foundDevice = found.emplace_back();
		for(std::size_t i = 0; i < serialSize - 1 /* omit '\0' */; i++)
			foundDevice.serial[i] = static_cast<char>(std::toupper(serial[i]));
		foundDevice.makeDriver = [](const device_eventhandler_t& r, neodevice_t& d) {
			return std::unique_ptr<SDIO>(new SDIO(r, d));
		};
	}
}

bool SDIO::open() {
	{
		auto socket = lockSocket();
		if(!(socket.writeTyped(RPC::SDIO_OPEN) && socket.writeString(device.device->getSerial())))
			return false;
		if(bool ret; !(socket.readTyped(ret) && ret))
			return false;

		{
			std::string mailboxName;
			if(!socket.readString(mailboxName))
				return false;
			if(!inboundIO.open(mailboxName))
				return false;
		}

		{
			std::string mailboxName;
			if(!socket.readString(mailboxName))
				return false;
			if(!outboundIO.open(mailboxName))
				return false;
		}
	}

	readThread = std::thread(&SDIO::readTask, this);
	writeThread = std::thread(&SDIO::writeTask, this);

	deviceOpen = true;

	return deviceOpen;
}

bool SDIO::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	closing = true;

	 // wait for the reader/writer threads to close
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// unblocks the reader/writer threads
	if(!inboundIO.close())
		return false;
	if(!outboundIO.close())
		return false;

	if(readThread.joinable())
		readThread.join();

	if(writeThread.joinable())
		writeThread.join();

	{
		auto socket = lockSocket();
		if(!socket.writeTyped(RPC::SDIO_CLOSE))
			return false;
		if(!socket.writeString(device.device->getSerial()))
			return false;
		if(bool ret; !(socket.readTyped(ret) && ret))
			return false;
	}

	uint8_t flush;
	WriteOperation flushop;
	while(readQueue.try_dequeue(flush)) {}
	while(writeQueue.try_dequeue(flushop)) {}

	closing = false;
	disconnected = false;
	deviceOpen = false;
	return true;
}

bool SDIO::isOpen() {
	return deviceOpen;
}

void SDIO::readTask() {
	uint8_t data[MAX_DATA_SIZE];
	uint16_t messageLength;
	while(!closing) {
		if(!inboundIO.read(data, messageLength, std::chrono::milliseconds(100))) {
			if(!inboundIO)
				break;
			continue;
		}

		if(messageLength > 0) {
			if(messageLength > MAX_DATA_SIZE) { // split message
				std::vector<uint8_t> reassembled(messageLength);
				std::memcpy(reassembled.data(), data, MAX_DATA_SIZE);
				auto offset = reassembled.data() + MAX_DATA_SIZE;
				for(auto remaining = messageLength - MAX_DATA_SIZE; remaining > 0; remaining -= messageLength) {
					if(!inboundIO.read(offset, messageLength, std::chrono::milliseconds(10))) {
						report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
						break;
					}
					offset += messageLength;
				}
				readQueue.enqueue_bulk(reassembled.data(), reassembled.size());
			} else {
				readQueue.enqueue_bulk(data, messageLength);
			}
		}
	}
}

void SDIO::writeTask() {
	WriteOperation writeOp;
	while(!closing && !isDisconnected()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		const auto dataSize = static_cast<LengthFieldType>(writeOp.bytes.size());

		const auto tryWrite = [&](const void* input, LengthFieldType length) -> bool {
			for(int i = 0; i < 50; ++i) { // try to write for 5s, making sure we can close if need be
				if(outboundIO.write(input, length, std::chrono::milliseconds(100)))
					return true;
				if(!outboundIO)
					return false;
			}
			disconnected = true;
			report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
			return false;
		};

		if(!tryWrite(writeOp.bytes.data(), dataSize))
			continue;

		if(writeOp.bytes.size() > MAX_DATA_SIZE) {
			auto offset = writeOp.bytes.data() + MAX_DATA_SIZE;
			for(LengthFieldType remaining = dataSize - MAX_DATA_SIZE; remaining > 0; ) {
				const auto toWrite = std::min(MAX_DATA_SIZE, remaining);
				if(!tryWrite(offset, toWrite))
					break;
				remaining -= toWrite;
				offset += toWrite;
			}
		}
	}
}
