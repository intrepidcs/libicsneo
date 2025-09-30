#include <sstream>
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/message/filter/main51messagefilter.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/device/device.h"
#include "icsneo/device/extensions/deviceextension.h"
#include "icsneo/disk/fat.h"
#include "icsneo/communication/message/filter/extendedresponsefilter.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // STL time functions
#endif

using namespace icsneo;

struct RTCCTIME {
	uint8_t FracSec;// --- fractions of seconds (00-99). Note that you can write only 0x00 here!
	uint8_t Sec;// --- Seconds (00-59)
	uint8_t Min;// --- (00-59)
	uint8_t Hour;// --- (00-23)
	uint8_t DOW;// --- (01-07)
	uint8_t Day;// --- (01-31)
	uint8_t Month;// --- (01-12)
	uint8_t Year;// --- (00-99)
};

static const uint8_t fromBase36Table[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

static const char toBase36Table[36] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
	'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

static const uint32_t toBase36Powers[7] = { 1, 36, 1296, 46656, 1679616, 60466176, 2176782336 };

#define MIN_BASE36_SERIAL (16796160)
#define MAX_SERIAL (2176782335)

std::string Device::SerialNumToString(uint32_t serial) {
	if(serial == 0 || serial > MAX_SERIAL)
		return "0";

	std::stringstream ss;
	if(serial >= MIN_BASE36_SERIAL) {
		for (auto i = 5; i >= 0; i--) {
			ss << toBase36Table[serial / toBase36Powers[i]];
			serial = serial % toBase36Powers[i];
		}
	} else {
		ss << serial;
	}
	return ss.str();
}

uint32_t Device::SerialStringToNum(const std::string& serial) {
	if(Device::SerialStringIsNumeric(serial)) {
		try {
			return std::stoi(serial);
		} catch(...) {
			return 0;
		}
	}

	if(serial.length() != 6)
		return 0; // Non-numeric serial numbers should be 6 characters

	uint32_t ret = 0;
	for (auto i = 0; i < 6; i++) {
		ret *= 36;
		ret += fromBase36Table[(unsigned char)serial[i]];
	}
	return ret;
}

bool Device::SerialStringIsNumeric(const std::string& serial) {
	if(serial.length() == 0)
		return false;

	if(serial.length() == 1)
		return isdigit(serial[0]);

	// Check the first two characters, at least one should be a character if we need to do a base36 conversion
	return isdigit(serial[0]) && isdigit(serial[1]);
}

Device::~Device() {
	if(isMessagePollingEnabled())
		disableMessagePolling();
	if(isOpen())
		close();
	if(heartbeatThread.joinable()) {
		stopHeartbeatThread = true;		
		heartbeatThread.join();
	}
}

uint16_t Device::getTimestampResolution() const {
	return com->decoder->timestampResolution;
}

std::string Device::describe() const {
	std::stringstream ss;
	ss << getProductName() << ' ' << getSerial();
	return ss.str();
}

bool Device::enableMessagePolling(std::optional<MessageFilter> filter) {
	if(isMessagePollingEnabled()) {// We are already polling
		report(APIEvent::Type::DeviceCurrentlyPolling, APIEvent::Severity::Error);
		return false;
	}
	if(!filter.has_value()) {
		// If no filter is provided, use a default that includes all messages
		filter.emplace(MessageFilter());
		filter->includeInternalInAny = true;
	}
	auto callback = std::make_shared<MessageCallback>(*filter, [this](std::shared_ptr<Message> message) {
		pollingContainer.enqueue(message);
		enforcePollingMessageLimit();
	});
	messagePollingCallbackID = com->addMessageCallback(callback);
	return true;
}

bool Device::disableMessagePolling() {
	if(!isMessagePollingEnabled()) {
		report(APIEvent::Type::DeviceNotCurrentlyPolling, APIEvent::Severity::Error);
		return false; // Not currently polling
	}
	auto ret = com->removeMessageCallback(messagePollingCallbackID);
	getMessages(); // Flush any messages still in the container
	messagePollingCallbackID = 0;
	return ret;
}

// Returns a pair of {vector, bool}, where the vector contains shared_ptrs to the returned msgs and the bool is whether or not the call was successful.
std::pair<std::vector<std::shared_ptr<Message>>, bool> Device::getMessages() {
	std::vector<std::shared_ptr<Message>> ret;
	bool retBool = getMessages(ret);
	return std::make_pair(ret, retBool);
}

std::shared_ptr<DeviceExtension> Device::getExtension(const std::string& name) const {
	std::shared_ptr<DeviceExtension> ret;
	std::lock_guard<std::mutex> lk(extensionsLock);
	for(auto& ext : extensions) {
		if((ext->getName() == name)) {
			ret = ext;
			break;
		}
	}
	return ret;
}

bool Device::getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit, std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	if(!isMessagePollingEnabled()) {
		report(APIEvent::Type::DeviceNotCurrentlyPolling, APIEvent::Severity::Error);
		return false;
	}

	// A limit of zero indicates no limit
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (pollingContainer.size_approx() + 4))
		limit = (pollingContainer.size_approx() + 4);

	if(container.size() < limit)
		container.resize(limit);

	size_t actuallyRead;
	if(timeout != std::chrono::milliseconds(0))
		actuallyRead = pollingContainer.wait_dequeue_bulk_timed(container.data(), limit, timeout);
	else
		actuallyRead = pollingContainer.try_dequeue_bulk(container.data(), limit);

	if(container.size() > actuallyRead)
		container.resize(actuallyRead);

	return true;
}

void Device::enforcePollingMessageLimit() {
	while(pollingContainer.size_approx() > pollingMessageLimit) {
		std::shared_ptr<Message> throwAway;
		pollingContainer.try_dequeue(throwAway);
		report(APIEvent::Type::PollingMessageOverflow, APIEvent::Severity::EventWarning);
	}
}

bool Device::refreshComponentVersions() {
	if(auto compVersions = com->getComponentVersionsSync()) {
		componentVersions = std::move(*compVersions);
		return true;
	}
	return false;
}

bool Device::open(OpenFlags flags, OpenStatusHandler handler) {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	if(!com->open()) {
		return false;
	}

	APIEvent::Type attemptErr = attemptToBeginCommunication();
	if(attemptErr != APIEvent::Type::NoErrorFound) {
		// We could not communicate with the device, let's see if an extension can
		bool tryAgain = false;
		forEachExtension([&tryAgain, &flags, &handler](const std::shared_ptr<DeviceExtension>& ext) -> bool {
			if(ext->onDeviceCommunicationDead(flags, handler))
				tryAgain = true;
			return true;
		});

		if(!tryAgain) {
			com->close();
			report(attemptErr, APIEvent::Severity::Error);
			return false; // Extensions couldn't save us
		}
		attemptErr = attemptToBeginCommunication();
		if(attemptErr != APIEvent::Type::NoErrorFound) {
			com->close();
			report(attemptErr, APIEvent::Severity::Error);
			return false;
		}
	}

	bool block = false;
	forEachExtension([&block, &flags, &handler](const std::shared_ptr<DeviceExtension>& ext) {
		if(ext->onDeviceOpen(flags, handler))
			return true;
		block = true;
		return false;
	});
	if(block) // Extensions say no
		return false;

	refreshComponentVersions();
	if(!settings->disabled) {
		// Since we will not fail the open if a settings read fails,
		// downgrade any errors to warnings. Otherwise the error will
		// go unnoticed in the opening thread's getLastError buffer.
		const bool downgrading = EventManager::GetInstance().isDowngradingErrorsOnCurrentThread();
		if(!downgrading)
			EventManager::GetInstance().downgradeErrorsOnCurrentThread();
		settings->refresh();
		if(!downgrading)
			EventManager::GetInstance().cancelErrorDowngradingOnCurrentThread();
	}

	MessageFilter filter;
	filter.includeInternalInAny = true;
	internalHandlerCallbackID = com->addMessageCallback(std::make_shared<MessageCallback>(filter, [this](std::shared_ptr<Message> message) {
		handleInternalMessage(message);
	}));

	// Clear the previous heartbeat thread, in case open() was called on this instance more than once
	if(heartbeatThread.joinable())
		heartbeatThread.join();

	stopHeartbeatThread = false;

	heartbeatThread = std::thread([this]() {
		EventManager::GetInstance().downgradeErrorsOnCurrentThread();

		MessageFilter filter;
		filter.includeInternalInAny = true;

		std::condition_variable heartbeatCV;
		std::mutex receivedMessageMutex;
		bool receivedMessage = false;
		auto messageReceivedCallbackID = com->addMessageCallback(std::make_shared<MessageCallback>(filter, [&](std::shared_ptr<Message>) {
			{
				std::scoped_lock<std::mutex> lk(receivedMessageMutex);
				receivedMessage = true;
			}
			heartbeatCV.notify_all();
		}));

		// Give the device time to get situated
		auto i = 150;
		while(!stopHeartbeatThread && i != 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			i--;
		}

		while(!stopHeartbeatThread) {
			std::unique_lock<std::mutex> recvLk(receivedMessageMutex);
			// Wait for 110ms for a possible heartbeat
			if(heartbeatCV.wait_for(recvLk, std::chrono::milliseconds(110), [&]() { return receivedMessage; })) {
				receivedMessage = false;
			} else if(!stopHeartbeatThread) { // Add this condition here in case the thread was stopped while waiting for the last message

				// Some communication, such as the bootloader and extractor interfaces, must
				// redirect the input stream from the device as it will no longer be in the
				// packet format we expect here. As a result, status updates will not reach
				// us here and suppressDisconnects() must be used. We don't want to request
				// a status and then redirect the stream, as we'll then be polluting an
				// otherwise quiet stream. This lock makes sure suppressDisconnects() will
				// block until we've either gotten our status update or disconnected from
				// the device.
				std::unique_lock<std::mutex> lk(heartbeatMutex);
				if(heartbeatSuppressed()) continue;

				// No heartbeat received, request a status
				com->sendCommand(Command::RequestStatusUpdate);

				// Check if we got a message, and if not, if settings are being applied
				if(heartbeatCV.wait_for(recvLk, std::chrono::milliseconds(3500), [&](){ return receivedMessage; })) {
					receivedMessage = false;
				} else {
					if(!stopHeartbeatThread && !isDisconnected()) {
						close();
						report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
					}
					break;
				}
			}
		}

		com->removeMessageCallback(messageReceivedCallbackID);
	});

	if(supportsLiveData())
		clearAllLiveData();

	return true;
}

APIEvent::Type Device::attemptToBeginCommunication() {
	versions.clear();

	if(!afterCommunicationOpen()) {
		// Very unlikely, at the time of writing this only fails if rawWrite does.
		// If you're looking for this error, you're probably looking for if(!serial) below.
		// "Communication could not be established with the device. Perhaps it is not powered with 12 volts?"
		return getCommunicationNotEstablishedError();
	}

	if(!enableNetworkCommunication(false))
		return getCommunicationNotEstablishedError();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	auto serial = com->getSerialNumberSync();
	int i = 0;
	while(!serial) {
		serial = com->getSerialNumberSync();
		if(i++ > 5)
			break;
	}

	if(!serial) // "Communication could not be established with the device. Perhaps it is not powered with 12 volts?"
		return getCommunicationNotEstablishedError();

	std::string currentSerial = getNeoDevice().serial;
	if(currentSerial != serial->deviceSerial)
		return APIEvent::Type::IncorrectSerialNumber;

	auto maybeVersions = com->getVersionsSync();
	if(!maybeVersions)
		return getCommunicationNotEstablishedError();
	else
		versions = std::move(*maybeVersions);

	// Get component versions before the extension "onDeviceOpen" hooks so that we can properly check verisons
	if(supportsComponentVersions()) {
		if(auto compVersions = com->getComponentVersionsSync())
			componentVersions = std::move(*compVersions);
		else
			report(APIEvent::Type::NotSupported, APIEvent::Severity::EventWarning); // outdated firmware
	}

	return APIEvent::Type::NoErrorFound;
}

bool Device::close() {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	stopHeartbeatThread = true;

	if (isMessagePollingEnabled()) {
		disableMessagePolling();
	}
	
	if(isOnline())
		goOffline();

	if(internalHandlerCallbackID)
		com->removeMessageCallback(internalHandlerCallbackID);

	internalHandlerCallbackID = 0;

	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onDeviceClose(); return true; });
	return com->close();
}

bool Device::enableLogData() {
	return com->sendCommand(Command::EnableLogData, true);
}

bool Device::disableLogData() {
	return com->sendCommand(Command::EnableLogData, false);
}

bool Device::goOnline() {
	static constexpr uint32_t onlineTimeoutMs = 5000;
	if(!enableNetworkCommunication(true, onlineTimeoutMs))
		return false;

	auto startTime = std::chrono::system_clock::now();

	ledState = LEDState::Online;

	updateLEDState();

	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::Reset_Status);
	filter->includeInternalInAny = true;

	// Wait until communication is enabled or 5 seconds, whichever comes first
	while((std::chrono::system_clock::now() - startTime) < std::chrono::seconds(5)) {
		if(latestResetStatus && latestResetStatus->comEnabled)
			break;

		bool failOut = false;
		com->waitForMessageSync([this, &failOut]() {
			if(!com->sendCommand(Command::RequestStatusUpdate)) {
				failOut = true;
				return false;
			}
			return true;
		}, filter, std::chrono::milliseconds(100));
		if(failOut)
			return false;
	}

	// (re)start the keeponline
	keeponline = std::make_unique<Periodic>([this] { return enableNetworkCommunication(true, onlineTimeoutMs); }, std::chrono::milliseconds(onlineTimeoutMs / 4));

	online = true;

	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onGoOnline(); return true; });

	return true;
}

bool Device::goOffline() {
	keeponline.reset();

	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onGoOffline(); return true; });

	if(isDisconnected()) {
		online = false;
		return true;
	}

	if(!enableNetworkCommunication(false))
		return false;

	ledState = (latestResetStatus && latestResetStatus->cmRunning) ? LEDState::CoreMiniRunning : LEDState::Offline;

	updateLEDState();

	online = false;

	return true;
}

int8_t Device::prepareScriptLoad() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::CoreMiniPreLoad);

	if(!com->sendCommand(Command::CoreMiniPreload))
		return false;

	int8_t retVal = 0;
	while(retVal == 0)
	{
		auto generic = com->waitForMessageSync(filter, std::chrono::milliseconds(1000));

		if(!generic) {
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
			return false;
		}

		const auto resp = std::static_pointer_cast<RawMessage>(generic);
		retVal = (int8_t)resp->data[0];
	}

	return retVal;
}

bool Device::startScript(Disk::MemoryType memType)
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	uint8_t location = static_cast<uint8_t>(memType);

	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::Device);
	filter->includeInternalInAny = true;

	const auto response = com->waitForMessageSync([&]() {
		return com->sendCommand(Command::LoadCoreMini, location);
	}, filter, std::chrono::milliseconds(2000));

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::stopScript()
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::Device);
	filter->includeInternalInAny = true;

	const auto response = com->waitForMessageSync([&]() {
		return com->sendCommand(Command::ClearCoreMini);
	}, filter);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::uploadCoremini(std::istream& stream, Disk::MemoryType memType) {

	if(stream.bad()) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}
	
	std::vector<char> bin(std::istreambuf_iterator<char>(stream), {}); // Read the whole stream

	if(bin.size() < 4) {
		report(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error);
		return false;
	}

	uint16_t scriptVersion = *(uint16_t*)(&bin[2]); // Third and fourth byte are version number stored in little endian

	auto scriptStatus = getScriptStatus();

	if(!scriptStatus) {
		return false; // Already added an API error
	}
	
	if(scriptStatus->coreminiVersion != scriptVersion) {
		// Version on device and script are not the same
		report(APIEvent::Type::CoreminiUploadVersionMismatch, APIEvent::Severity::Error);
		return false;
	}

	auto startAddress = getCoreminiStartAddress(memType);
	if(!startAddress) {
		return false;
	}

	auto connected = isLogicalDiskConnected();
	
	if(!connected) {
		return false; // Already added an API error
	}
	
	if(!(*connected)) {
		report(APIEvent::Type::DiskNotConnected, APIEvent::Severity::Error);
		return false;
	}

	if(!stopScript()) {
		return false;
	}

	if(!clearScript(memType)) {
		return false;
	}


	if(!eraseScriptMemory(memType, static_cast<uint64_t>(bin.size()))) {
		return false;
	}

	auto numWritten = writeLogicalDisk(*startAddress, (uint8_t*)bin.data(), static_cast<uint64_t>(bin.size()), std::chrono::milliseconds(2000), memType);

	if(!numWritten) {
		return false; // Already added an API error
	}
	
	if(*numWritten != static_cast<uint64_t>(bin.size())) {
		report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
		return false; // Failed to write
	}

	return true;
}

bool Device::eraseScriptMemory(Disk::MemoryType memType, uint64_t amount) {
	static std::shared_ptr<MessageFilter> NeoEraseDone = std::make_shared<MessageFilter>(Network::NetID::NeoMemoryWriteDone);

	if(!supportsEraseMemory()) {
		return true;
	}

	auto startAddress = getCoreminiStartAddress(memType);
	if(!startAddress) {
		return false;
	}

	if(memType != Disk::MemoryType::Flash) {
		// Only need to erase on flash
		return true;
	}

	std::vector<uint8_t> arguments(9, 0);

	uint32_t numWords = static_cast<uint32_t>(amount / 2);

	arguments[0] = static_cast<uint8_t>(memType);
	*reinterpret_cast<uint32_t*>(&arguments[1]) = static_cast<uint32_t>(*startAddress / 512);
	*reinterpret_cast<uint32_t*>(&arguments[5]) = numWords;

	auto msg = com->waitForMessageSync([this, &arguments] {
		return com->sendCommand(Command::NeoEraseMemory, arguments);
	}, NeoEraseDone, std::chrono::milliseconds(3000));

	if(!msg) {
		return false;
	}

	return true;
}

bool Device::clearScript(Disk::MemoryType memType)
{
	if(!stopScript())
		return false;
	

	auto startAddress = getCoreminiStartAddress(memType);
	if(!startAddress) {
		return false;
	}

	std::vector<uint8_t> clearData(512, 0xCD);
	auto written = writeLogicalDisk(*startAddress, clearData.data(), clearData.size(), std::chrono::milliseconds(2000), memType);

	if(!written) {
		return false;
	}
	if(*written == 0) {
		return false;
	}

	return true;
}

std::optional<CoreminiHeader> Device::readCoreminiHeader(Disk::MemoryType memType) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	auto startAddress = getCoreminiStartAddress(memType);
	if(!startAddress) {
		return std::nullopt;
	}

	auto connected = isLogicalDiskConnected();
	
	if(!connected) {
		return std::nullopt; // Already added an API error
	}

	#pragma pack(push, 2)
	struct RawCoreminiHeader {
		uint16_t fileType;
		uint16_t fileVersion;
		uint32_t storedFileSize;
		uint32_t fileChecksum;
		union
		{
			struct
			{
				uint32_t skipDecompression : 1;
				uint32_t encryptedMode : 1;
				uint32_t reserved : 30;
			} bits;
			uint32_t word;
		} flags;
		uint8_t fileHash[32];
		union
		{
			struct
			{
				uint32_t lsb;
				uint32_t msb;
			} words;
			uint64_t time64;
		} createTime;
		uint8_t reserved[8];
	};
	#pragma pack(pop)

	RawCoreminiHeader header = {};
	auto numRead = readLogicalDisk(*startAddress, (uint8_t*)&header, sizeof(header), std::chrono::milliseconds(2000), memType);

	if(!numRead) {
		return std::nullopt; // Already added an API error
	}
	
	if(*numRead != sizeof(header)) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(header.fileType != 0x0907) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::optional<CoreminiHeader> ret;
	ret.emplace();
	ret->coreminiVersion = header.fileVersion;
	ret->storedFileSize = header.storedFileSize;
	ret->fileChecksum = header.fileChecksum;
	ret->skipDecompression = static_cast<bool>(header.flags.bits.skipDecompression);
	ret->encryptedMode = static_cast<bool>(header.flags.bits.encryptedMode);
	std::copy(std::begin(header.fileHash), std::end(header.fileHash), ret->fileHash.begin());
	static constexpr std::chrono::seconds icsEpochDelta(1167609600);
	static constexpr uint8_t timestampResolution = 25;
	static constexpr uint16_t nsInUs = 1'000;
	ret->timestamp += icsEpochDelta + std::chrono::microseconds(header.createTime.time64 * timestampResolution / nsInUs);
	return ret;
}

bool Device::transmit(std::shared_ptr<Frame> frame) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	if(!isSupportedTXNetwork(frame->network)) {
		report(APIEvent::Type::UnsupportedTXNetwork, APIEvent::Severity::Error);
		return false;
	}

	bool extensionHookedTransmit = false;
	bool transmitStatusFromExtension = false;
	forEachExtension([&](const std::shared_ptr<DeviceExtension>& ext) {
		if(!ext->transmitHook(frame, transmitStatusFromExtension))
			extensionHookedTransmit = true;
		return !extensionHookedTransmit; // false breaks out of the loop early
	});
	if(extensionHookedTransmit)
		return transmitStatusFromExtension;

	std::vector<uint8_t> packet;
	if(!com->encoder->encode(*com->packetizer, packet, frame))
		return false;

	return com->sendPacket(packet);
}

bool Device::transmit(std::vector<std::shared_ptr<Frame>> frames) {
	for(auto& frame : frames) {
		if(!transmit(frame))
			return false;
	}
	return true;
}

void Device::setWriteBlocks(bool blocks) {
	com->setWriteBlocks(blocks);
}

size_t Device::getNetworkCountByType(Network::Type type) const {
	size_t count = 0;
	for(const auto& net : getSupportedRXNetworks())
		if(net.getType() == type)
			count++;
	return count;
}

// Indexed starting at one
Network Device::getNetworkByNumber(Network::Type type, size_t index) const {
	size_t count = 0;
	for(const auto& net : getSupportedRXNetworks()) {
		if(net.getType() == type) {
			count++;
			if(count == index)
				return net;
		}
	}
	return Network::NetID::Invalid;
}

std::shared_ptr<HardwareInfo> Device::getHardwareInfo(std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullptr;
	}
	auto filter = std::make_shared<MessageFilter>(Message::Type::HardwareInfo);

	auto response = com->waitForMessageSync([this]() {
		return com->sendCommand(Command::GetHardwareInfo);
	}, filter, timeout);

	if(!response) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return nullptr;
	}

	auto hardwareInfo = std::dynamic_pointer_cast<HardwareInfo>(response);
	if(!hardwareInfo) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return nullptr;
	}

	return hardwareInfo;
}


std::optional<uint64_t> Device::readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, Disk::MemoryType memType) {
	if(!into || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lk(diskMutex);

	if(diskReadDriver->getAccess() == Disk::Access::EntireCard && diskWriteDriver->getAccess() == Disk::Access::VSA) {
		// We have mismatched drivers, we need to add an offset to the diskReadDriver
		const auto offset = Disk::FindVSAInFAT([this, &timeout, &memType](uint64_t pos, uint8_t *into, uint64_t amount) {
			const auto start = std::chrono::steady_clock::now();
			auto ret = diskReadDriver->readLogicalDisk(*com, report, pos, into, amount, timeout, memType);
			timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
			return ret;
		});
		if(!offset.has_value())
			return std::nullopt;
		diskReadDriver->setVSAOffset(*offset);
	}

	return diskReadDriver->readLogicalDisk(*com, report, pos, into, amount, timeout, memType);
}

std::optional<uint64_t> Device::writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout, Disk::MemoryType memType) {
	if(!from || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return diskWriteDriver->writeLogicalDisk(*com, report, *diskReadDriver, pos, from, amount, timeout, memType);
}

std::optional<bool> Device::isLogicalDiskConnected() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	const auto info = com->getLogicalDiskInfoSync();
	if(!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return info->connected;
}

std::optional<uint64_t> Device::getLogicalDiskSize() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	const auto info = com->getLogicalDiskInfoSync();
	if(!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return std::nullopt;
	}

	const auto reportedSize = info->getReportedSize();

	if(diskReadDriver->getAccess() == Disk::Access::VSA)
		return reportedSize - diskReadDriver->getVSAOffset();

	return reportedSize;
}

std::optional<uint64_t> Device::getVSAOffsetInLogicalDisk() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(diskReadDriver->getAccess() == Disk::Access::VSA || diskReadDriver->getAccess() == Disk::Access::None)
		return 0ull;
	
	auto offset = Disk::FindVSAInFAT([this](uint64_t pos, uint8_t *into, uint64_t amount) {
		return diskReadDriver->readLogicalDisk(*com, report, pos, into, amount);
	});
	if(!offset.has_value())
		return std::nullopt;

	if(diskReadDriver->getAccess() == Disk::Access::EntireCard && diskWriteDriver->getAccess() == Disk::Access::VSA) {
		// We have mismatched drivers, we need to add an offset to the diskReadDriver
		diskReadDriver->setVSAOffset(*offset);
		return 0ull;
	}
	return *offset;
}

std::optional<bool> Device::getDigitalIO(IO type, size_t number /* = 1 */) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
		if(getEthernetActivationLineCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!ethActivationStatus.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return ethActivationStatus;
	case IO::USBHostPower:
		if(getUSBHostPowerCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!usbHostPowerStatus.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return usbHostPowerStatus;
	case IO::BackupPowerEnabled:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!backupPowerEnabled.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return backupPowerEnabled;
	case IO::BackupPowerGood:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!backupPowerGood.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return backupPowerGood;
	case IO::Misc: {
		bool found = false;
		for(const auto& misc : getMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsDigitalIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscDigital.size())
			break; // ParameterOutOfRange

		if(!miscDigital[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscDigital[number - 1];
	}
	case IO::EMisc: {
		bool found = false;
		for(const auto& misc : getEMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsDigitalIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscDigital.size())
			break; // ParameterOutOfRange

		// If there is ever a device with overlapping misc IOs and emisc IOs,
		// you will need to make a new member variable for the emisc IOs.
		if(!miscDigital[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscDigital[number - 1];
	}
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return std::nullopt;
}

bool Device::setDigitalIO(IO type, size_t number, bool value) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
		if(getEthernetActivationLineCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		ethActivationStatus = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(1), uint8_t(value ? 1 : 0), // enetActivateSet, enetActivateValue
			uint8_t(0), uint8_t(0), // usbHostPowerSet, usbHostPowerValue
			uint8_t(0), uint8_t(0) // backupPowerSet, backupPowerValue
		});
	case IO::USBHostPower:
		if(getUSBHostPowerCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		usbHostPowerStatus = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(0), uint8_t(0), // enetActivateSet, enetActivateValue
			uint8_t(1), uint8_t(value ? 1 : 0), // usbHostPowerSet, usbHostPowerValue
			uint8_t(0), uint8_t(0) // backupPowerSet, backupPowerValue
		});
	case IO::BackupPowerEnabled:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		backupPowerEnabled = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(0), uint8_t(0), // enetActivateSet, enetActivateValue
			uint8_t(0), uint8_t(value ? 1 : 0), // usbHostPowerSet, usbHostPowerValue (set to work around firmware bug)
			uint8_t(1), uint8_t(value ? 1 : 0) // backupPowerSet, backupPowerValue
		});
	case IO::BackupPowerGood:
		break; // Read-only, return ParameterOutOfRange
	case IO::Misc:
	case IO::EMisc:
		break; // Read-only for the moment, return ParameterOutOfRange
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return false;
}

std::optional<double> Device::getAnalogIO(IO type, size_t number /* = 1 */) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
	case IO::USBHostPower:
	case IO::BackupPowerEnabled:
	case IO::BackupPowerGood:
		break;
	case IO::Misc: {
		bool found = false;
		for(const auto& misc : getMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsAnalogIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscAnalog.size())
			break; // ParameterOutOfRange

		if(!miscAnalog[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscAnalog[number - 1];
	}
	case IO::EMisc: {
		bool found = false;
		for(const auto& misc : getEMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsAnalogIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscAnalog.size())
			break; // ParameterOutOfRange

		// If there is ever a device with overlapping misc IOs and emisc IOs,
		// you will need to make a new member variable for the emisc IOs.
		if(!miscAnalog[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscAnalog[number - 1];
	}
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return std::nullopt;
}

void Device::wiviThreadBody() {
	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	std::unique_lock<std::mutex> lk(wiviMutex);
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	bool first = true;
	while(!stopWiVIThread) {
		if(first) // Skip the first wait
			first = false;
		else
			stopWiVIcv.wait_for(lk, std::chrono::seconds(3));

		// Use the command GetAll to get a WiVI::Info structure from the device
		const auto generic = com->waitForMessageSync([this]() {
			return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::GetAll::Encode());
		}, filter, std::chrono::milliseconds(1000));

		if(!generic || generic->type != Message::Type::WiVICommandResponse) {
			report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
			continue;
		}

		const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
		if(!resp->success || !resp->info.has_value()) {
			report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
			continue;
		}
		
		// Now we know we have a WiVI::Info structure

		// Don't process captures unless there is a callback attached,
		// we don't want to clear any while nobody's listening.
		bool processCaptures = false;
		for(const auto& cb : newCaptureCallbacks) {
			if(cb) {
				processCaptures = true;
				break;
			}
		}

		if(processCaptures) {
			std::vector<uint8_t> clearMasks;
			for (size_t i = 0; i < resp->info->captures.size(); i++) {
				const auto capture = resp->info->captures.at(i);

				if(capture.flags.uploadOverflow)
					report(APIEvent::Type::WiVIUploadStackOverflow, APIEvent::Severity::Error);

				const auto MaxUploads = sizeof(capture.uploadStack) / sizeof(capture.uploadStack[0]);
				auto uploadCount = capture.flags.uploadStackSize + 1u;
				if(uploadCount > MaxUploads) {
					report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
					uploadCount = MaxUploads;
				}

				for(size_t j = 0; j < uploadCount; j++) {
					const auto& upload = capture.uploadStack[j];
					if(!upload.flags.pending)
						continue; // Not complete yet, don't notify

					// Schedule this upload to be cleared from the firmware's stack
					if(clearMasks.size() != resp->info->captures.size())
						clearMasks.resize(resp->info->captures.size());
					clearMasks[i] |= (1 << j);

					WiVIUpload wiviUpload {};
					wiviUpload.captureIndex = capture.captureBlockIndex;
					wiviUpload.cellular = capture.flags.uploadOverCellular;
					wiviUpload.wifi = capture.flags.uploadOverWiFi;
					wiviUpload.isPrePost = capture.flags.isPrePost;
					wiviUpload.isPreTime = capture.flags.isPreTime;
					wiviUpload.preTriggerSize = capture.preTriggerSize;
					wiviUpload.priority = capture.flags.uploadPriority;
					wiviUpload.startSector = upload.startSector;
					wiviUpload.endSector = upload.endSector;

					// Notify the client
					for(const auto& cb : newCaptureCallbacks) {
						if(cb) {

							lk.unlock();
							try {
								cb(wiviUpload);
							} catch(...) {
								report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
							}
							lk.lock();
						}
					}
				}
			}

			if(!clearMasks.empty()) {
				const auto clearMasksGenericResp = com->waitForMessageSync([this, &clearMasks]() {
					return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::ClearUploads::Encode(clearMasks));
				}, filter, std::chrono::milliseconds(1000));

				if(!clearMasksGenericResp
					|| clearMasksGenericResp->type != Message::Type::WiVICommandResponse
					|| !std::static_pointer_cast<WiVI::ResponseMessage>(clearMasksGenericResp)->success)
				{
					report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
				}
			}
		}

		// Process sleep requests
		if(resp->info->sleepRequest & 1 /* sleep requested by VSSAL */) {
			// Notify any callers we haven't notified yet
			for(auto& cb : sleepRequestedCallbacks) {
				if(!cb.second && cb.first) {
					cb.second = true;
					lk.unlock();
					try {
						cb.first(resp->info->connectionTimeoutMinutes);
					} catch(...) {
						report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
					}
					lk.lock();
				}
			}
		} else {
			// If the sleepRequest becomes 1 again we will notify again
			for(auto& cb : sleepRequestedCallbacks)
				cb.second = false;
		}
	}
}

void Device::stopWiVIThreadIfNecessary(std::unique_lock<std::mutex> lk) {
	// The callbacks will be empty std::functions if they are removed
	for(const auto& cb : newCaptureCallbacks) {
		if(cb)
			return; // We still need the WiVI Thread
	}

	for(const auto& cb : sleepRequestedCallbacks) {
		if(cb.first)
			return; // We still need the WiVI Thread
	}

	stopWiVIThread = true;
	lk.unlock();
	stopWiVIcv.notify_all();
	wiviThread.join();
	wiviThread = std::thread();
}

Lifetime Device::addNewCaptureCallback(NewCaptureCallback cb) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(wiviMutex);
	if(!wiviThread.joinable()) {
		// Start the thread
		stopWiVIThread = false;
		wiviThread = std::thread([this]() { wiviThreadBody(); });
	}

	size_t idx = 0;
	for(; idx < newCaptureCallbacks.size(); idx++) {
		if(!newCaptureCallbacks[idx]) // Empty space (previously erased callback)
			break;
	}

	if(idx == newCaptureCallbacks.size()) // Create a new space
		newCaptureCallbacks.push_back(std::move(cb));
	else
		newCaptureCallbacks[idx] = std::move(cb);

	// Cleanup function to remove this capture callback
	return Lifetime([this, idx]() {
		// TODO: Hold a weak ptr to the `this` instead of relying on the user to keep `this` valid
		std::unique_lock<std::mutex> lk2(wiviMutex);
		newCaptureCallbacks[idx] = NewCaptureCallback();
		stopWiVIThreadIfNecessary(std::move(lk2));
	});
}

Lifetime Device::addSleepRequestedCallback(SleepRequestedCallback cb) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(wiviMutex);
	if(!wiviThread.joinable()) {
		// Start the thread
		stopWiVIThread = false;
		wiviThread = std::thread([this]() { wiviThreadBody(); });
	}

	size_t idx = 0;
	for(; idx < sleepRequestedCallbacks.size(); idx++) {
		if(!sleepRequestedCallbacks[idx].first) // Empty space (previously erased callback)
			break;
	}

	if(idx == sleepRequestedCallbacks.size()) // Create a new space
		sleepRequestedCallbacks.emplace_back(std::move(cb), false);
	else
		sleepRequestedCallbacks[idx] = { std::move(cb), false };

	// Cleanup function to remove this sleep requested callback
	return Lifetime([this, idx]() {
		// TODO: Hold a weak ptr to the `this` instead of relying on the user to keep `this` valid
		std::unique_lock<std::mutex> lk2(wiviMutex);
		sleepRequestedCallbacks[idx].first = SleepRequestedCallback();
		stopWiVIThreadIfNecessary(std::move(lk2));
	});
}

std::optional<bool> Device::isSleepRequested() const {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return std::nullopt;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	// Hold this lock so the WiVI stack doesn't issue a WiVICommand at the same time as us
	std::lock_guard<std::mutex> lk(wiviMutex);
	const auto generic = com->waitForMessageSync([this]() {
		// VSSAL sets bit0 to indicate that it's waiting to sleep, then
		// it waits for Wireless neoVI to acknowledge by clearing it.
		// If we set bit1 at the same time we clear bit0, remote wakeup
		// will be suppressed (assuming the device supported it in the
		// first place)
		return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::GetSignal::Encode(WiVI::SignalType::SleepRequest));
	}, filter, std::chrono::milliseconds(1000));

	if(!generic || generic->type != Message::Type::WiVICommandResponse) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
	if(!resp->success || !resp->value.has_value()) {
		report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return *resp->value;
}

bool Device::allowSleep(bool remoteWakeup) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return false;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	// Hold this lock so the WiVI stack doesn't issue a WiVICommand at the same time as us
	std::lock_guard<std::mutex> lk(wiviMutex);
	const auto generic = com->waitForMessageSync([this, remoteWakeup]() {
		// VSSAL sets bit0 to indicate that it's waiting to sleep, then
		// it waits for Wireless neoVI to acknowledge by clearing it.
		// If we set bit1 at the same time we clear bit0, remote wakeup
		// will be suppressed (assuming the device supported it in the
		// first place)
		return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::SetSignal::Encode(
			WiVI::SignalType::SleepRequest, remoteWakeup ? 0 : 2
		));
	}, filter, std::chrono::milliseconds(1000));

	if(!generic || generic->type != Message::Type::WiVICommandResponse) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
	if(!resp->success) {
		report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

void Device::scriptStatusThreadBody()
{
	std::unique_lock<std::mutex> lk(scriptStatusMutex);

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	bool first = true;
	while(!stopScriptStatusThread)
	{
		if(first) // Skip the first wait
			first = false;
		else
			stopScriptStatusCv.wait_for(lk, std::chrono::seconds(10));

		const auto resp = getScriptStatus();

		if(!resp)
			continue;

		//If value changed/was inserted, notify callback
		if(updateScriptStatusValue(ScriptStatus::CoreMiniRunning, resp->isCoreminiRunning))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniRunning, resp->isCoreminiRunning);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::IsEncrypted, resp->isEncrypted))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::IsEncrypted, resp->isEncrypted);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::SectorOverflow, resp->sectorOverflows))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::SectorOverflow, resp->sectorOverflows);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::RemainingSectors, resp->numRemainingSectorBuffers))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::RemainingSectors, resp->numRemainingSectorBuffers);
			lk.lock();
		}

		bool logging = false;
		if(updateScriptStatusValue(ScriptStatus::LastSector, resp->lastSector))
		{
			logging = true;
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::LastSector, resp->lastSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::Logging, logging))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::Logging, logging);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::ReadBinSize, resp->readBinSize))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::ReadBinSize, resp->readBinSize);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MinSector, resp->minSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MinSector, resp->minSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MaxSector, resp->maxSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MaxSector, resp->maxSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CurrentSector, resp->currentSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CurrentSector, resp->currentSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniCreateTime, resp->coreminiCreateTime))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniCreateTime, resp->coreminiCreateTime);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::FileChecksum, resp->fileChecksum))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::FileChecksum, resp->fileChecksum);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniVersion, resp->coreminiVersion))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniVersion, resp->coreminiVersion);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniHeaderSize, resp->coreminiHeaderSize))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniHeaderSize, resp->coreminiHeaderSize);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::DiagnosticErrorCode, resp->diagnosticErrorCode))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::DiagnosticErrorCode, resp->diagnosticErrorCode);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::DiagnosticErrorCodeCount, resp->diagnosticErrorCodeCount))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::DiagnosticErrorCodeCount, resp->diagnosticErrorCodeCount);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MaxCoreMiniSize, resp->maxCoreminiSizeKB))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MaxCoreMiniSize, resp->maxCoreminiSizeKB);
			lk.lock();
		}
	}
}

std::shared_ptr<ScriptStatusMessage> Device::getScriptStatus() const
{
	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::ScriptStatus);

	const auto generic = com->waitForMessageSync([this]() {
		return com->sendCommand(Command::ScriptStatus);
	}, filter, std::chrono::milliseconds(3000));

	if(!generic || generic->type != Message::Type::ScriptStatus) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return nullptr;
	}

	return std::static_pointer_cast<ScriptStatusMessage>(generic);
}

bool Device::updateScriptStatusValue(ScriptStatus key, uint64_t value)
{
	auto pair = scriptStatusValues.find(key);
	if(pair != scriptStatusValues.end())
	{
		if(pair->second != value)
		{
			//Value changed
			scriptStatusValues[key] = value;
			return true;
		}
		//Value didn't change
		return false;
	}

	//Value was inserted
	scriptStatusValues.insert(std::make_pair(key, value));
	return true;
}

void Device::notifyScriptStatusCallback(ScriptStatus key, uint64_t value)
{
	auto callbackList = scriptStatusCallbacks.find(key);
	if(callbackList != scriptStatusCallbacks.end())
	{
		for(const auto& callback : callbackList->second)
		{
			if(callback) {
				try {
					callback(value);
				} catch(...) {
					report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
				}
			}
		}
	}
}

Lifetime Device::addScriptStatusCallback(ScriptStatus key, ScriptStatusCallback cb)
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(scriptStatusMutex);
	if(!scriptStatusThread.joinable()) {
		// Start the thread
		stopScriptStatusThread = false;
		scriptStatusThread = std::thread([this]() { scriptStatusThreadBody(); });
	}

	size_t idx = 0;
	std::vector<ScriptStatusCallback> callbackList;
	auto callbackPair = scriptStatusCallbacks.find(key);
	if(callbackPair != scriptStatusCallbacks.end())
		callbackList = callbackPair->second;

	if(idx == callbackList.size())
		callbackList.push_back(std::move(cb));
	else callbackList[idx] = std::move(cb);

	scriptStatusCallbacks.insert_or_assign(key, callbackList);

	return Lifetime([this, key, idx](){
		std::unique_lock<std::mutex> lk2(scriptStatusMutex);
		auto callbackList = scriptStatusCallbacks.find(key);
		if(callbackList != scriptStatusCallbacks.end())
			callbackList->second[idx] = ScriptStatusCallback();
		stopScriptStatusThreadIfNecessary(std::move(lk2));
	});
}

void Device::stopScriptStatusThreadIfNecessary(std::unique_lock<std::mutex> lk)
{
	for(const auto& callbackList : scriptStatusCallbacks)
	{
		for(const auto& callback : callbackList.second)
		{
			if(callback)
				return;
		}
	}

	stopScriptStatusThread = true;
	lk.unlock();
	stopScriptStatusCv.notify_all();
	scriptStatusThread.join();
	scriptStatusThread = std::thread();
}

Lifetime Device::suppressDisconnects() {
	std::lock_guard<std::mutex> lk(heartbeatMutex);
	heartbeatSuppressedByUser++;
	return Lifetime([this] { 
		std::lock_guard<std::mutex> lk2(heartbeatMutex);
		heartbeatSuppressedByUser--;
	});
}

void Device::addExtension(std::shared_ptr<DeviceExtension>&& extension) {
	std::lock_guard<std::mutex> lk(extensionsLock);
	extensions.push_back(extension);
}

void Device::forEachExtension(std::function<bool(const std::shared_ptr<DeviceExtension>&)> fn) {
	std::vector<std::shared_ptr<DeviceExtension>> extensionsCopy;

	{
		std::lock_guard<std::mutex> lk(extensionsLock);
		extensionsCopy = extensions;
	}

	for(const auto& ext : extensionsCopy) {
		if(!fn(ext))
			break;
	}
}

void Device::handleInternalMessage(std::shared_ptr<Message> message) {
	switch(message->type) {
		case Message::Type::ResetStatus:
			latestResetStatus = std::static_pointer_cast<ResetStatusMessage>(message);
			break;
		case Message::Type::RawMessage: {
			auto rawMessage = std::static_pointer_cast<RawMessage>(message);
			switch(rawMessage->network.getNetID()) {
				case Network::NetID::DeviceStatus:
					// Device Status format is unique per device, so the devices need to decode it themselves
					handleDeviceStatus(rawMessage);
					break;
				default:
					break; //std::cout << "HandleInternalMessage got a message from " << message->network << " and it was unhandled!" << std::endl;
			}
			break;
		}
		case Message::Type::Frame: {
			// Device is not guaranteed to be a CANMessage, it might be a RawMessage
			// if it couldn't be decoded to a CANMessage. We only care about the
			// CANMessage decoding right now.
			auto canmsg = std::dynamic_pointer_cast<CANMessage>(message);
			if(canmsg)
				handleNeoVIMessage(std::move(canmsg));
			break;
		}
		default: break;
	}
	forEachExtension([&](const std::shared_ptr<DeviceExtension>& ext) {
		ext->handleMessage(message);
		return true; // false breaks out early
	});
}

void Device::handleNeoVIMessage(std::shared_ptr<CANMessage> message) {
	switch(message->arbid) {
		case 0x103: { // Report Message (neoVI FIRE 2)
			if(message->data.size() < 34) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);
				return;
			}

			uint16_t emisc[2];
			memcpy(emisc, message->data.data() + 24, sizeof(emisc));
			std::lock_guard<std::mutex> lk(ioMutex);
			miscAnalog[0] = (message->data[24] | (uint16_t(message->data[25]) << 8)) * 0.01015511; // In volts now
			miscAnalog[1] = (message->data[26] | (uint16_t(message->data[27]) << 8)) * 0.01015511;
			miscDigital[0] = message->data[28] & 0x01;
			miscDigital[1] = message->data[29] & 0x01;
			miscDigital[4] = message->data[30] & 0x01;
			miscDigital[5] = message->data[31] & 0x01;
		}
	}
}

bool Device::firmwareUpdateSupported() {
	bool ret = false;
	forEachExtension([&ret](const std::shared_ptr<DeviceExtension>& ext) {
		if(ext->providesFirmware()) {
			ret = true;
			return false;
		}
		return true; // false breaks out early
	});
	return ret;
}

APIEvent::Type Device::getCommunicationNotEstablishedError() {
	if(firmwareUpdateSupported()) {
		if(requiresVehiclePower())
			return APIEvent::Type::NoSerialNumberFW12V;
		else
			return APIEvent::Type::NoSerialNumberFW;
	} else {
		if(requiresVehiclePower())
			return APIEvent::Type::NoSerialNumber12V;
		else
			return APIEvent::Type::NoSerialNumber;
	}
}

void Device::updateLEDState() {
	std::vector<uint8_t> args {(uint8_t) ledState};
	com->sendCommand(Command::UpdateLEDState, args);
}

std::optional<EthPhyMessage> Device::sendEthPhyMsg(const EthPhyMessage& message, std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}
	if(!getEthPhyRegControlSupported()) {
		report(APIEvent::Type::EthPhyRegisterControlNotAvailable, APIEvent::Severity::Error);
		return std::nullopt;
	}
	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::vector<uint8_t> bytes;
	HardwareEthernetPhyRegisterPacket::EncodeFromMessage(message, bytes, report);
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, bytes](){ return com->sendCommand(Command::PHYControlRegisters, bytes); },
		std::make_shared<MessageFilter>(Message::Type::EthernetPhyRegister), timeout);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<EthPhyMessage>(response);
	if(!retMsg) {
		return std::nullopt;
	}
	return std::make_optional<EthPhyMessage>(*retMsg);
}

std::optional<bool> Device::SetRootDirectoryEntryFlags(uint8_t mask, uint8_t values, uint32_t collectionEntryByteAddress)
{
	if(!supportsWiVI())
	{
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::EventWarning);
		return std::nullopt;
	}

	if (mask & RootDirectoryEntryFlags::IsPrePost)
	{
		report(APIEvent::Type::RestrictedEntryFlag, APIEvent::Severity::EventWarning);
		mask &= ~RootDirectoryEntryFlags::IsPrePost;
		values &= ~RootDirectoryEntryFlags::IsPrePost;
	}

	if (mask & RootDirectoryEntryFlags::PrePostTriggered)
	{
		report(APIEvent::Type::RestrictedEntryFlag, APIEvent::Severity::EventWarning);
		mask &= ~RootDirectoryEntryFlags::PrePostTriggered;
		values &= ~RootDirectoryEntryFlags::PrePostTriggered;
	}

	auto timeout = std::chrono::milliseconds(2500);
	std::vector<uint8_t> args(
		{(uint8_t)(collectionEntryByteAddress & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 8) & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 16) & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 24) & 0xFF),
		 values,
		 mask});

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, args](){ return com->sendCommand(ExtendedCommand::SetRootFSEntryFlags, args); },
		std::make_shared<MessageFilter>(Message::Type::ExtendedResponse), timeout);
	if(!response)
	{
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<ExtendedResponseMessage>(response);
	if(!retMsg)
	{
		// TODO fix this error
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}
	bool success = retMsg->response == ExtendedResponse::OK;
	if(!success)
	{
		// TODO fix this error
		report(APIEvent::Type::Unknown, APIEvent::Severity::EventWarning);
	}
	// Valid device with a properly formed response, return success
	return success;
}

std::optional<std::chrono::time_point<std::chrono::system_clock>> Device::getRTC()
{
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::RED_GET_RTC);
	std::shared_ptr<Message> generic = com->waitForMessageSync([this]() {
		return com->sendCommand(Command::GetRTC);
	}, filter, std::chrono::milliseconds(3000));
	if(!generic) // Did not receive a message
		return std::nullopt;

	auto rawMes = std::dynamic_pointer_cast<RawMessage>(generic);
	if(!rawMes)
		return std::nullopt;

	if(rawMes->data.size() != sizeof(RTCCTIME))
		return std::nullopt;

	const auto* time = (RTCCTIME*)rawMes->data.data();
	std::tm stdTime = {};
	// std::tm has no member for the `FracSec` member of RTCCTIME struct
	stdTime.tm_sec = time->Sec;
	stdTime.tm_min = time->Min;
	stdTime.tm_hour = time->Hour;
	stdTime.tm_mday = time->Day;
	stdTime.tm_mon = time->Month - 1; // [0-11]
	stdTime.tm_year = time->Year + 100; // Number of years since 1900+100
	stdTime.tm_wday = time->DOW;
	// RTCCTIME struct has no member for `tm_yday`

	#ifdef _MSC_VER
		#define timegm _mkgmtime
	#endif

	return std::chrono::system_clock::from_time_t(timegm(&stdTime));
}

bool Device::setRTC(const std::chrono::time_point<std::chrono::system_clock>& time)
{
	auto now = std::chrono::system_clock::to_time_t(time);
	const auto timeInfo = std::gmtime(&now);
	if(!timeInfo)
		return false;

	// Populate the RTCCTIME struct using the timeInfo and offsets
	// Create a vector of arguments to send as the payload to the communication command
	std::vector<uint8_t> bytestream(sizeof(RTCCTIME));
	auto rtcVals = (RTCCTIME*)bytestream.data();
	rtcVals->FracSec = (uint8_t)0x00;
	rtcVals->Sec = (uint8_t)timeInfo->tm_sec;
	rtcVals->Min = (uint8_t)timeInfo->tm_min;
	rtcVals->Hour = (uint8_t)timeInfo->tm_hour;
	rtcVals->DOW = (uint8_t)timeInfo->tm_wday + 1;
	rtcVals->Day = (uint8_t)timeInfo->tm_mday;
	rtcVals->Month = (uint8_t)timeInfo->tm_mon + 1; // [0-11]
	rtcVals->Year = (uint8_t)timeInfo->tm_year % 100; // divide by 100 and take remainder to get last 2 digits of year

	const auto generic = com->waitForMessageSync([&]() {
		return com->sendCommand(Command::SetRTC, bytestream);
	}, std::make_shared<Main51MessageFilter>(Command::SetRTC), std::chrono::milliseconds(100));

	if(!generic) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	auto m51msg = std::dynamic_pointer_cast<Main51Message>(generic);
	if(!m51msg || m51msg->data.empty() || m51msg->data.size() > 2) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return false;
	}

	return m51msg->data.front();
}

std::optional<std::set<SupportedFeature>> Device::getSupportedFeatures() {
	auto timeout = std::chrono::milliseconds(100);
	std::shared_ptr<Message> msg = com->waitForMessageSync(
		[this](){ return com->sendCommand(ExtendedCommand::GetSupportedFeatures, {}); },
		std::make_shared<MessageFilter>(Message::Type::SupportedFeatures), timeout);	
	if(!msg) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	const auto& typedResponse = std::dynamic_pointer_cast<SupportedFeaturesMessage>(msg);
	if(!typedResponse) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	return std::move(typedResponse->features);
}

std::optional<size_t> Device::getGenericBinarySize(uint16_t binaryIndex) {
	auto timeout = std::chrono::milliseconds(2000);
	
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::vector<uint8_t> args = GenericBinaryStatusPacket::EncodeArguments(binaryIndex);
	
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, &args](){ 
			return com->sendCommand(ExtendedCommand::GenericBinaryInfo, args); 
		},
		std::make_shared<MessageFilter>(Message::Type::GenericBinaryStatus),
		timeout
	);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<GenericBinaryStatusMessage>(response);
	if(!retMsg) {
		return std::nullopt;
	}

	return retMsg->binarySize;
}

bool Device::readBinaryFile(std::ostream& stream, uint16_t binaryIndex) {
	auto timeout = std::chrono::milliseconds(100);

	auto size = getGenericBinarySize(binaryIndex);

	if(!size) {
		return false;
	}

	std::vector<uint8_t> arguments(sizeof(ExtendedDataMessage::ExtendedDataHeader));
	ExtendedDataMessage::ExtendedDataHeader& parameters = *reinterpret_cast<ExtendedDataMessage::ExtendedDataHeader*>(arguments.data());	

	auto filter = std::make_shared<MessageFilter>(Network::NetID::ExtendedData);
	
	for(size_t offset = 0; offset < *size; offset+=ExtendedDataMessage::MaxExtendedDataBufferSize) {
		parameters.subCommand = ExtendedDataSubCommand::GenericBinaryRead;
		parameters.userValue = static_cast<uint32_t>(binaryIndex);
		parameters.offset = static_cast<uint32_t>(offset);
		parameters.length = static_cast<uint32_t>(std::min(ExtendedDataMessage::MaxExtendedDataBufferSize, *size - offset));

		std::shared_ptr<Message> response = com->waitForMessageSync(
			[this, arguments](){ 
				return com->sendCommand(Command::ExtendedData, arguments); 
			},
			filter, 
			timeout
		);
		if(!response) {
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
			return false;
		}
		auto retMsg = std::static_pointer_cast<ExtendedDataMessage>(response);

		if(!stream.write(reinterpret_cast<char*>(retMsg->data.data()), retMsg->data.size())) {
			return false;
		}
	}
	return true;
}

bool Device::writeBinaryFile(const std::vector<uint8_t>& in, uint16_t binaryIndex)
{
	auto timeout = std::chrono::milliseconds(100);

	auto size = in.size();

	std::vector<uint8_t> arguments(sizeof(ExtendedDataMessage::ExtendedDataHeader) + ExtendedDataMessage::MaxExtendedDataBufferSize);
	ExtendedDataMessage::ExtendedDataHeader& parameters = *reinterpret_cast<ExtendedDataMessage::ExtendedDataHeader*>(arguments.data());

	auto filter = std::make_shared<MessageFilter>(Network::NetID::ExtendedData);

	for (size_t offset = 0; offset < size; offset += ExtendedDataMessage::MaxExtendedDataBufferSize)
	{
		parameters.subCommand = ExtendedDataSubCommand::GenericBinaryWrite;
		parameters.userValue = static_cast<uint32_t>(binaryIndex);
		parameters.offset = static_cast<uint32_t>(offset);
		parameters.length = static_cast<uint32_t>(std::min(ExtendedDataMessage::MaxExtendedDataBufferSize, size - offset));
		(void)memcpy(&arguments[sizeof(ExtendedDataMessage::ExtendedDataHeader)], &in[offset], parameters.length);

		std::shared_ptr<Message> response = com->waitForMessageSync(
			[this, arguments]() {
				return com->sendCommand(Command::ExtendedData, arguments);
			},
			filter,
			timeout
		);
		if (!response) {
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
			return false;
		}
	}
	return true;
}

bool Device::subscribeLiveData(std::shared_ptr<LiveDataCommandMessage> message) {
	if(!supportsLiveData()) {
		report(APIEvent::Type::LiveDataNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}
	if((message->args.size() > MAX_LIVE_DATA_ENTRIES) || message->args.empty()) {
		report(APIEvent::Type::LiveDataInvalidArgument, APIEvent::Severity::Error);
		return false;
	}

	std::vector<uint8_t> bytes;
	if(!com->encoder->encode(*com->packetizer, bytes, message)) {
		report(APIEvent::Type::LiveDataEncoderError, APIEvent::Severity::Error);
		return false;
	}

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, &bytes](){ return com->sendPacket(bytes); },
		std::make_shared<MessageFilter>(Message::Type::LiveData));

	if(response) {
		auto statusMsg = std::dynamic_pointer_cast<LiveDataStatusMessage>(response);
		if(statusMsg && statusMsg->requestedCommand == message->cmd) {
			switch(statusMsg->status) {
				case LiveDataStatus::SUCCESS:
					return true;
				case LiveDataStatus::ERR_DUPLICATE:
				case LiveDataStatus::ERR_HANDLE:
				{
					report(APIEvent::Type::LiveDataInvalidHandle, APIEvent::Severity::Error);
					return false;
				}
				case LiveDataStatus::ERR_FULL:
				{
					report(APIEvent::Type::LiveDataMaxSignalsReached, APIEvent::Severity::Error);
					return false;
				}
				case LiveDataStatus::ERR_UNKNOWN_COMMAND:
				{
					report(APIEvent::Type::LiveDataCommandFailed, APIEvent::Severity::Error);
					return false;
				}
				default:
					break;
			}
		}
	}
	report(APIEvent::Type::LiveDataNoDeviceResponse, APIEvent::Severity::Error);
	return false;
}

bool Device::unsubscribeLiveData(const LiveDataHandle& handle) {
	if(!supportsLiveData()) {
		report(APIEvent::Type::LiveDataNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}
	if(!handle) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}
	auto msg = std::make_shared<LiveDataMessage>();
	msg->cmd = LiveDataCommand::UNSUBSCRIBE;
	msg->handle = handle;
	std::vector<uint8_t> bytes;
	if(!com->encoder->encode(*com->packetizer, bytes, msg)) {
		report(APIEvent::Type::LiveDataEncoderError, APIEvent::Severity::Error);
		return false;
	}

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, &bytes](){ return com->sendPacket(bytes); },
		std::make_shared<MessageFilter>(Message::Type::LiveData));

	if(!response) {
		report(APIEvent::Type::LiveDataNoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	auto statusMsg = std::dynamic_pointer_cast<LiveDataStatusMessage>(response);
	if(!statusMsg || statusMsg->requestedCommand != msg->cmd) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return false;
	}

	if(statusMsg->status != LiveDataStatus::SUCCESS) {
		report(APIEvent::Type::LiveDataCommandFailed, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::clearAllLiveData() {
	if(!supportsLiveData()) {
		report(APIEvent::Type::LiveDataNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	auto msg = std::make_shared<LiveDataMessage>();
	msg->cmd = LiveDataCommand::CLEAR_ALL;
	std::vector<uint8_t> bytes;
	if(!com->encoder->encode(*com->packetizer, bytes, msg)) {
		report(APIEvent::Type::LiveDataEncoderError, APIEvent::Severity::Error);
		return false;
	}
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, &bytes](){ return com->sendPacket(bytes); },
		std::make_shared<MessageFilter>(Message::Type::LiveData));

	if(!response) {
		report(APIEvent::Type::LiveDataNoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	auto statusMsg = std::dynamic_pointer_cast<LiveDataStatusMessage>(response);
	if(!statusMsg || statusMsg->requestedCommand != msg->cmd) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return false;
	}

	if(statusMsg->status != LiveDataStatus::SUCCESS) {
		report(APIEvent::Type::LiveDataCommandFailed, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::setValueLiveData(std::shared_ptr<LiveDataSetValueMessage> message) {
	if(!supportsLiveData()) {
		report(APIEvent::Type::LiveDataNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}
	if((message->args.size() != message->values.size()) || message->args.empty()) {
		report(APIEvent::Type::LiveDataInvalidArgument, APIEvent::Severity::Error);
		return false;
	}

	std::vector<uint8_t> bytes;
	if(!com->encoder->encode(*com->packetizer, bytes, message)) {
		report(APIEvent::Type::LiveDataEncoderError, APIEvent::Severity::Error);
		return false;
	}

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, &bytes](){ return com->sendPacket(bytes); },
		std::make_shared<MessageFilter>(Message::Type::LiveData));

	if(response) {
		auto statusMsg = std::dynamic_pointer_cast<LiveDataStatusMessage>(response);
		if(statusMsg && statusMsg->requestedCommand == message->cmd) {
			switch(statusMsg->status) {
				case LiveDataStatus::SUCCESS:
					return true;
				case LiveDataStatus::ERR_DUPLICATE:
				case LiveDataStatus::ERR_HANDLE:
				{
					report(APIEvent::Type::LiveDataInvalidHandle, APIEvent::Severity::Error);
					return false;
				}
				case LiveDataStatus::ERR_FULL:
				{
					report(APIEvent::Type::LiveDataMaxSignalsReached, APIEvent::Severity::Error);
					return false;
				}
				case LiveDataStatus::ERR_UNKNOWN_COMMAND:
				{
					report(APIEvent::Type::LiveDataCommandFailed, APIEvent::Severity::Error);
					return false;
				}
				default:
					break;
			}
		}
	}
	report(APIEvent::Type::LiveDataNoDeviceResponse, APIEvent::Severity::Error);
	return false;
}

bool Device::readVSA(const VSAExtractionSettings& extractionSettings) {
	if(isOnline()) {
		goOffline();
	}
	auto innerReadVSA = [&](uint64_t diskSize) -> bool {
		// Adjust driver to offset to start of VSA file
		const auto& offset = getVSAOffsetInLogicalDisk();
		if(!offset) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		diskReadDriver->setVSAOffset(*offset);

		// Gather metadata about VSA file system
		VSAMetadata metadata;

		metadata.diskSize = diskSize;

		if(!probeVSA(metadata, extractionSettings)) {
			return false;
		}

		if(extractionSettings.filters.empty()) { // Full SD Dump
			if(!parseVSA(metadata, extractionSettings)) {
				return false;
			}
		} else { // Only read data specified by filters
			for(const auto& filter : extractionSettings.filters) {
				if(!parseVSA(metadata, extractionSettings, filter)) {
					return false;
				}
			}
		}
		return true;
	};

	auto bufferSize = getVSADiskSize();

	if(!bufferSize) {
		return false;
	}

	const auto wasScriptStarted = getScriptStatus()->isCoreminiRunning;

	if(extractionSettings.stopCoreMini && wasScriptStarted) {
		stopScript();
	}

	const auto ret = innerReadVSA(*bufferSize);

	// Restart CoreMini script if stopped
	if(extractionSettings.stopCoreMini && wasScriptStarted) {
		startScript();
	}
	return ret;
}

bool Device::probeVSA(VSAMetadata& metadata, const VSAExtractionSettings& extractionSettings) {
	auto cmTimestamp = getCoreMiniScriptTimestamp();
	if(!metadata.coreMiniTimestamp) {
		return false;
	}
	metadata.coreMiniTimestamp = *cmTimestamp;

	const auto& isOverlapped = isVSAOverlapped(metadata);
	if(isOverlapped) {
		metadata.isOverlapped = *isOverlapped;
	} else {
		return false;
	}

	if(!findFirstVSARecord(metadata.firstRecordLocation, metadata.firstRecord, extractionSettings, metadata)) {
		return false;
	}

	if(!findLastVSARecord(metadata.lastRecordLocation, metadata.lastRecord, extractionSettings, metadata)) {
		return false;
	}

	if(metadata.isOverlapped) {
		// The last byte in the buffer should immediately precede the first if the buffer is overlapped
		metadata.bufferEnd = metadata.firstRecordLocation;
	} else {
		metadata.bufferEnd = metadata.lastRecordLocation;
		const auto& type = metadata.lastRecord->getType();
		if(type == VSA::Type::AA0D || type == VSA::Type::AA0E || type == VSA::Type::AA0F) {
			// Add bytes based off of how many records should be in extended record sequence
			metadata.bufferEnd += std::dynamic_pointer_cast<VSAExtendedMessage>(metadata.lastRecord)->getRecordCount() * VSA::StandardRecordSize;
		} else if(type == VSA::Type::AA6A) {
			// Add a full sector for script status backup records
			metadata.bufferEnd += Disk::SectorSize;
		} else {
			// All other records add only one single record offset.
			metadata.bufferEnd += VSA::StandardRecordSize;
		}
	}
	return true;
}

bool Device::findFirstVSARecord(uint64_t& firstOffset, std::shared_ptr<VSA>& firstRecord, 
	const VSAExtractionSettings& extractionSettings, std::optional<VSAMetadata> optMetadata) {
	// Grab important metadata features if metadata not defined
	VSAMetadata metadata;
	if(!optMetadata) {
		const auto& coreMiniTimestamp = getCoreMiniScriptTimestamp();
		if(!coreMiniTimestamp) {
			return false;
		}
		metadata.coreMiniTimestamp = *coreMiniTimestamp;
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return false;
		}
		metadata.diskSize = *diskSize;
		const auto& isOverlapped = isVSAOverlapped(metadata);
		if(isOverlapped) {
			metadata.isOverlapped = *isOverlapped;
		} else {
			return false;
		}
	} else {
		metadata = *optMetadata;
	}

	if(!metadata.isOverlapped) { // Grab the first record in the buffer
		std::vector<uint8_t> buffer;
		buffer.resize(Disk::SectorSize);
		const auto& bytesRead = readLogicalDisk(VSA::RecordStartOffset, buffer.data(), Disk::SectorSize);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		VSAParser parser(report);
		std::shared_ptr<VSA> record;
		const auto& firstRecordStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, record);
		if(firstRecordStatus == VSAParser::RecordParseStatus::Success) {
			firstOffset = VSA::RecordStartOffset;
			firstRecord = record;
			return true;
		}
		report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
		return false;
	}

	static constexpr size_t RegionCount = 2; // Number of regions to chunk remaining disk into during logarithmic search
	static constexpr size_t LinearSearchMaxSize = 0x00010000u; // Size of remaining disk space at which to begin iterative search (64 KB)
	
	// Initialize variables for disk search
	VSAParser parser(report);
	uint64_t first = VSA::RecordStartOffset; // First byte of vsa records to search
	uint64_t last = metadata.diskSize; // One beyond the last byte of vsa records to search
	uint64_t lastSector = last - Disk::SectorSize;
	std::vector<uint8_t> buffer;
	buffer.resize(Disk::SectorSize);
	uint64_t start = first; // First byte of data to search for this iteration of algorithm
	uint64_t stop = last; // One beyond last byte of data to search for this iteration of algorithm

	// Repeatedly chunk data into regions and find largest negative difference in timestamp
	// (i.e., where the timestamp at the beginning of the region is much larger than at the end)
	// until data is under the LinearSearchMaxSize
	while(stop - start > LinearSearchMaxSize) {
		uint64_t regionSize = (stop - start) / RegionCount;
		regionSize -= regionSize % Disk::SectorSize; // Ensures regions are chunked on a sector boundary
		uint64_t largestNegative = 0;
		size_t largestNegativeIndex = 0;
		uint64_t smallestStartTimestamp = UINT64_MAX;
		size_t smallestStartTimestampIndex = 0; // Only necessary in cases where smallest timestamp falls on front edge of a region
		for(size_t i = 0; i < RegionCount; i++) {
			uint64_t regionStart = start + i * regionSize;
			// Start of last sector to read in the current region
			uint64_t regionLastSector = (i != RegionCount - 1)
											? regionStart + regionSize - Disk::SectorSize
											: (stop - Disk::SectorSize) - ((stop - Disk::SectorSize) % Disk::SectorSize);
			const auto& timestampStart = getVSATimestampOrBefore(parser, buffer, regionStart, first, metadata);
			const auto& timestampStop = getVSATimestampOrAfter(parser, buffer, regionLastSector, lastSector, metadata);
			// Ensure valid timestamps were found
			if(!timestampStart || !timestampStop) {
				return false;
			}
			// Check if region has largest negative
			if(*timestampStart > *timestampStop && *timestampStart - *timestampStop > largestNegative) {
				if(*timestampStop > metadata.coreMiniTimestamp || extractionSettings.parseOldRecords) {
					largestNegative = *timestampStart - *timestampStop;
					largestNegativeIndex = i;
				}
			}
			// Track smallest start timestamp for edge case
			if(*timestampStart < smallestStartTimestamp) {
				if(*timestampStart >= metadata.coreMiniTimestamp || extractionSettings.parseOldRecords) {
					smallestStartTimestamp = *timestampStart;
					smallestStartTimestampIndex = i;
				}
			}
		}

		if(largestNegative == 0) {
			// We did not find a switch between large and small timestamps within a region.
			// Therefore the smallest timestamp is the first record within one of the regions.
			uint64_t location = start + smallestStartTimestampIndex * regionSize;
			const auto& bytesRead = readLogicalDisk(location, buffer.data(), Disk::SectorSize);
			if(!bytesRead || *bytesRead < Disk::SectorSize) {
				report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
				return false;
			}
			std::shared_ptr<VSA> record;
			parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, record);
			if(record) {
				firstRecord = record;
				firstOffset = location;
				return true;
			}
			report(APIEvent::Type::VSAByteParseFailure, APIEvent::Severity::Error);
			return false;
		}

		// Set start and stop to be start and stop of largest negative region
		start += largestNegativeIndex * regionSize;
		stop = (largestNegativeIndex != RegionCount - 1) ? start + regionSize : stop;
	}

	// Initialize variables for iterative search
	auto smallestTimestampLocation = UINT64_MAX;
	auto smallestTimestamp = UINT64_MAX;
	size_t regionSize = static_cast<size_t>(stop - start - ((stop - start) % Disk::SectorSize));
	buffer.resize(regionSize);
	const auto& bytesRead = readLogicalDisk(start, buffer.data(), regionSize);
	if(!bytesRead || *bytesRead < regionSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return false;
	}
	std::shared_ptr<VSA> minRecord;

	// Iteratively find sector with smallest timestamp from largest negative region
	for(size_t offset = 0; offset + VSA::StandardRecordSize < regionSize; offset += VSA::StandardRecordSize) {
		std::shared_ptr<VSA> record;
		auto recordParseStatus = parser.getRecordFromBytes(buffer.data() + offset, Disk::SectorSize, record);

		if(recordParseStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
			// Backtrack to get timestamp if first record is consecutive extended record
			auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(record);
			auto pos = start + offset;
			if(!findFirstExtendedVSAFromConsecutive(extendedRecord, pos, parser, metadata)) {
				// Possibly erased by looped buffer
				continue;
			}
			uint64_t timestamp = extendedRecord->getTimestamp();
			if(timestamp < smallestTimestamp) {
				smallestTimestampLocation = pos;
				smallestTimestamp = timestamp;
				minRecord = extendedRecord;
			}
		} else if(record) {
			// Update data tracking record with minimum timestamp
			uint64_t timestamp = record->getTimestamp();
			if(timestamp < smallestTimestamp) {
				smallestTimestampLocation = offset + start;
				smallestTimestamp = timestamp;
				minRecord = record;
			}
		}
	}
	if(smallestTimestamp == UINT64_MAX || !minRecord) {
		report(APIEvent::Type::VSATimestampNotFound, APIEvent::Severity::Error);
		return false;
	}
	firstOffset = smallestTimestampLocation;
	firstRecord = minRecord;
	return true;
}

bool Device::findLastVSARecord(uint64_t& lastOffset, std::shared_ptr<VSA>& lastRecord, 
	const VSAExtractionSettings& extractionSettings, std::optional<VSAMetadata> optMetadata) {
	static constexpr auto LinearSearchSize = 0x8000u;

	// Grab important metadata features if metadata not defined
	VSAMetadata metadata;
	if(optMetadata) {
		metadata = *optMetadata;
	} else {
		const auto& coreMiniTimestamp = getCoreMiniScriptTimestamp();
		if(!coreMiniTimestamp) {
			return false;
		}
		metadata.coreMiniTimestamp = *coreMiniTimestamp;
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return false;
		}
		metadata.diskSize = *diskSize;
		const auto& isOverlapped = isVSAOverlapped(metadata);
		if(!isOverlapped) {
			return false;
		}
		metadata.isOverlapped = *isOverlapped;
	}

	// Find record prior to first (chronological) record if VSA buffer is overlapped
	VSAParser parser(report);
	if(metadata.isOverlapped) {
		uint64_t firstOffset;
		std::shared_ptr<VSA> firstRecord;
		if(metadata.firstRecordLocation != UINT64_MAX && metadata.firstRecord) {
			firstOffset = metadata.firstRecordLocation;
		} else if(!findFirstVSARecord(firstOffset, firstRecord, extractionSettings, metadata)) {
			return false;
		}
		std::vector<uint8_t> buffer;
		buffer.resize(Disk::SectorSize);
		// Read sector before first if buffer is looped
		const auto& bytesRead = vsaReadLogicalDisk(firstOffset - Disk::SectorSize, buffer.data(), Disk::SectorSize, metadata);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		uint16_t bufferOffset = Disk::SectorSize;
		do {
			bufferOffset -= VSA::StandardRecordSize;
			const auto& lastRecordStatus = parser.getRecordFromBytes(buffer.data() + bufferOffset, VSA::StandardRecordSize, lastRecord);
			if(lastRecordStatus == VSAParser::RecordParseStatus::Success) {
				lastOffset = firstOffset - Disk::SectorSize + bufferOffset;
				return true;
			} else if(lastRecordStatus != VSAParser::RecordParseStatus::NotARecordStart) {
				// Reverse search for record with valid timestamp
				// Necessary if previous record is a pad record or consecutive extended record
				auto pos = firstOffset - bufferOffset;
				if(findPreviousRecordWithTimestamp(lastRecord, pos, parser)) {
					lastOffset = pos;
					return true;
				}
				return true;
			}
		} while(bufferOffset > 0);
		lastRecord = nullptr;
		report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
		return false;
	}

	// Binary search for last record until region is less than LinearSearchSize
	std::vector<uint8_t> buffer;
	buffer.resize(Disk::SectorSize);
	uint64_t left = VSA::RecordStartOffset;
	uint64_t right = metadata.diskSize;
	while(right - left > LinearSearchSize) {
		uint64_t pos = (left + right) / 2;
		pos -= pos % Disk::SectorSize;
		const auto& bytesRead = readLogicalDisk(pos, buffer.data(), Disk::SectorSize);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		std::shared_ptr<VSA> testRecord;
		const auto& testStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, testRecord);
		if(testStatus == VSAParser::RecordParseStatus::NotARecordStart) {
			right = pos;
		} else if(testStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
			auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(testRecord);
			auto tempPos = pos;
			if(!findFirstExtendedVSAFromConsecutive(extendedRecord, tempPos, parser, metadata)) {
				tempPos = pos;
				if(!findPreviousRecordWithTimestamp(testRecord, tempPos, parser)) {
					return false;
				}
				if(testRecord->getTimestamp() > metadata.coreMiniTimestamp || extractionSettings.parseOldRecords) {
					left = tempPos;
				} else {
					right = tempPos;
				}
			}
			if(extendedRecord->getTimestamp() > metadata.coreMiniTimestamp || extractionSettings.parseOldRecords) {
				left = tempPos;
			} else {
				right = tempPos;
			}
		} else {
			if(testRecord->getTimestamp() > metadata.coreMiniTimestamp || extractionSettings.parseOldRecords) {
				left = pos;
			} else {
				right = pos;
			}
		}
	}

	buffer.resize(static_cast<size_t>(LinearSearchSize));
	const auto& bytesRead = readLogicalDisk(left, buffer.data(), LinearSearchSize);
	if(!bytesRead || *bytesRead < LinearSearchSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return false;
	}
	std::shared_ptr<VSA> previousRecord;
	auto previousParseStatus = parser.getRecordFromBytes(buffer.data(), LinearSearchSize, previousRecord);
	uint64_t previousParseLocation = left;
	uint64_t bufferOffset = VSA::StandardRecordSize;

	// Ensure we have a record with a valid timestamp as the previousParseResult
	while(previousParseStatus != VSAParser::RecordParseStatus::Success) {
		if(bufferOffset >= LinearSearchSize) { // Never found a record with a valid timestamp (unlikely)
			report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
			return false;
		}
		previousParseStatus = parser.getRecordFromBytes(buffer.data() + bufferOffset, static_cast<size_t>(LinearSearchSize - bufferOffset), previousRecord);
		previousParseLocation = left + bufferOffset;
		bufferOffset += VSA::StandardRecordSize;
	}

	// Perform linear search for last record
	while(bufferOffset < LinearSearchSize) {
		std::shared_ptr<VSA> record;
		auto parseStatus = parser.getRecordFromBytes(buffer.data() + bufferOffset, static_cast<size_t>(LinearSearchSize - bufferOffset), record);
		if(parseStatus == VSAParser::RecordParseStatus::NotARecordStart) {
			// We found the end of the VSA buffer
			// Return the last valid record we found
			lastOffset = previousParseLocation;
			lastRecord = previousRecord;
			return true;
		} else if(
			parseStatus == VSAParser::RecordParseStatus::Success &&
			record->getTimestamp() < metadata.coreMiniTimestamp &&
			!extractionSettings.parseOldRecords
		) { // We have entered outdated record data
			lastOffset = previousParseLocation;
			lastRecord = previousRecord;
			return true;
		} else if(parseStatus == VSAParser::RecordParseStatus::Success) {
			// Save new last-record data and update bufferOffset according to record size
			previousParseStatus = parseStatus;
			previousRecord = record;
			previousParseLocation = left + bufferOffset;
			bufferOffset += (record->getType() == VSA::Type::AA6A) ? Disk::SectorSize : VSA::StandardRecordSize;
		} else {
			bufferOffset += VSA::StandardRecordSize;
		}
	}
	report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
	return false; // Somehow we did not find any non-record data despite non-overlapped buffer
}

std::optional<bool> Device::isVSAOverlapped(std::optional<VSAMetadata> optMetadata) {
	// Grab important metadata features if metadata not defined
	VSAMetadata metadata;
	if(!optMetadata) {
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return std::nullopt;
		}
		metadata.diskSize = *diskSize;
		const auto& coreMiniTimestamp = getCoreMiniScriptTimestamp();
		if(!coreMiniTimestamp) {
			return std::nullopt;
		}
		metadata.coreMiniTimestamp = *coreMiniTimestamp;
	} else {
		metadata = *optMetadata;
	}

	// Read first sector
	VSAParser parser(report);
	std::vector<uint8_t> buffer;
	buffer.resize(Disk::SectorSize);
	const auto& bytesReadFirst = readLogicalDisk(VSA::RecordStartOffset, buffer.data(), Disk::SectorSize);
	if(!bytesReadFirst || *bytesReadFirst < Disk::SectorSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return std::nullopt;
	}
	std::shared_ptr<VSA> firstRecord;
	auto firstRecordStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, firstRecord);
	if(firstRecordStatus == VSAParser::RecordParseStatus::NotARecordStart) {
		report(APIEvent::Type::VSABufferCorrupted, APIEvent::Severity::Error);
		return std::nullopt; // Beginning of buffer is not a record
	} else if(firstRecordStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
		return true; // The only way to have a consecutive extended record at the beginning is if the buffer looped
	}

	// Read last sector
	uint64_t lastPos = metadata.diskSize - Disk::SectorSize;
	lastPos -= lastPos % Disk::SectorSize;
	const auto& bytesReadLast = readLogicalDisk(lastPos, buffer.data(), Disk::SectorSize);
	if(!bytesReadLast || *bytesReadLast < Disk::SectorSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return std::nullopt;
	}
	std::shared_ptr<VSA> lastSectorRecord;
	auto lastSectorStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, lastSectorRecord);
	if(lastSectorStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
		// Find the beginning record of the extended record sequence
		auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(lastSectorRecord);
		if(!findFirstExtendedVSAFromConsecutive(extendedRecord, lastPos, parser, metadata)) {
			return std::nullopt;
		}
		return firstRecord->getTimestamp() >= extendedRecord->getTimestamp() &&
			   extendedRecord->getTimestamp() > metadata.coreMiniTimestamp;
	} else if(firstRecord && lastSectorRecord && firstRecord->isTimestampValid() && lastSectorRecord->isTimestampValid()) {
		// Handle situation where both first and last records have valid timestamps
		return firstRecord->getTimestamp() >= lastSectorRecord->getTimestamp() &&
			   lastSectorRecord->getTimestamp() > metadata.coreMiniTimestamp;
	} else if(lastSectorStatus == VSAParser::RecordParseStatus::NotARecordStart) {
		// The vsa record buffer is not full
		report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
		return false;
	}
	report(APIEvent::Type::VSABufferFormatError, APIEvent::Severity::Error);
	return std::nullopt;
}

bool Device::findFirstExtendedVSAFromConsecutive(std::shared_ptr<VSAExtendedMessage>& record, uint64_t& pos, VSAParser& parser, std::optional<VSAMetadata> optMetadata) {
	static constexpr auto MaxReadAttempts = 10;

	VSAMetadata metadata;
	if(!optMetadata) {
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return false;
		}
		metadata.diskSize = *diskSize;
	} else {
		metadata = *optMetadata;
	}

	// Reverse iteratively search from given pos for first record in sequence
	const auto& index = record->getIndex();
	const auto& seqNum = record->getSequenceNum();
	pos -= (index - 1) * VSA::StandardRecordSize;
	std::vector<uint8_t> buffer;
	buffer.resize(Disk::SectorSize);
	uint16_t readCount = 0;
	while(readCount < MaxReadAttempts) {
		const auto& bytesRead = vsaReadLogicalDisk(pos, buffer.data(), Disk::SectorSize, metadata);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		std::shared_ptr<VSA> possibleFirstRecord;
		const auto& possibleFirstStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, possibleFirstRecord);
		std::shared_ptr<VSAExtendedMessage> extendedVSA;
		if(possibleFirstStatus == VSAParser::RecordParseStatus::NotARecordStart) {
			// Most likely either null bytes or the middle of an AA6A record
			pos -= VSA::StandardRecordSize;
			readCount++;
			continue;
		}
		extendedVSA = std::dynamic_pointer_cast<VSAExtendedMessage>(possibleFirstRecord);
		if(!extendedVSA) {
			// Record is not an extended message record
			pos -= VSA::StandardRecordSize;
			readCount++;
			continue;
		}
		if(possibleFirstStatus == VSAParser::RecordParseStatus::Success && extendedVSA->getSequenceNum() == seqNum) {
			// Found the desired record
			record = extendedVSA;
			return true;
		} else if(possibleFirstStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
			if(seqNum != extendedVSA->getSequenceNum()) { // Another extended record sequence is intermixed with the one we want
				pos -= VSA::StandardRecordSize;
			} else { // Traverse backwards the minimum amount the first record must be behind extendedVSA
				pos -= extendedVSA->getIndex() * VSA::StandardRecordSize;
			}
		}
		readCount++;
	}
	report(APIEvent::Type::VSAMaxReadAttemptsReached, APIEvent::Severity::Error);
	return false;
}

bool Device::findPreviousRecordWithTimestamp(std::shared_ptr<VSA>& record, uint64_t& pos, VSAParser& parser) {
	static constexpr uint16_t MaxReadAttempts = 100;
	static constexpr uint64_t ReadSize = 0x1000;

	std::vector<uint8_t> buffer;
	buffer.resize(ReadSize);
	uint16_t readCount = 0;
	while(readCount < MaxReadAttempts) {
		pos -= ReadSize;
		const auto& bytesRead = vsaReadLogicalDisk(pos, buffer.data(), ReadSize);
		if(!bytesRead || *bytesRead < ReadSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		// Reverse search through buffer for a record with a valid timestamp
		for (size_t offsetBufferEnd = VSA::StandardRecordSize; offsetBufferEnd < ReadSize; offsetBufferEnd += VSA::StandardRecordSize) {
			const auto& parseStatus = parser.getRecordFromBytes(buffer.data() + ReadSize - offsetBufferEnd, offsetBufferEnd, record);
			if(parseStatus == VSAParser::RecordParseStatus::Success) {
				pos -= offsetBufferEnd;
				return true;
			}
		}
		readCount++;
	}
	report(APIEvent::Type::VSAMaxReadAttemptsReached, APIEvent::Severity::Error);
	return false; // Exit if record not found within readCount number of reads
}

bool Device::findVSAOffsetFromTimepoint(ICSClock::time_point point, uint64_t& vsaOffset, std::shared_ptr<VSA>& record, 
	const VSAExtractionSettings& extractionSettings, std::optional<VSAMetadata> optMetadata) {
	static constexpr uint64_t LinearSearchTickDifference = 100000ull; // The maximum number of ticks offset from point at which to do a linear search

	// Grab important metadata features if metadata not defined
	VSAMetadata metadata;
	if(!optMetadata) {
		if(!probeVSA(metadata, extractionSettings)) {
			return false;
		}
	} else {
		metadata = *optMetadata;
	}
	if(metadata.diskSize == 0) { // Disk size is unknown
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	uint64_t firstOffset = metadata.firstRecordLocation; // Offset of the first record chronologically
	const auto& desiredTimestamp = VSA::getICSTimestampFromTimepoint(point);
	VSAParser parser(report);
	std::vector<uint8_t> buffer;
	buffer.resize(Disk::SectorSize);

	if(desiredTimestamp <= metadata.firstRecord->getTimestamp()) {
		// Timestamp is less than first timestamp so we just return the first
		vsaOffset = metadata.firstRecordLocation;
		record = metadata.firstRecord;
		return true;
	}

	if(desiredTimestamp > metadata.lastRecord->getTimestamp()) {
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	// Handle situation where first offset is not on sector boundary
	// Find smallest tick difference between record in sector and desired timestamp
	uint64_t bestTickDiff = UINT64_MAX;
	std::shared_ptr<VSA> bestRecord = nullptr;
	uint64_t bestOffset = UINT64_MAX;

	auto findBestTickDiff = [&] (uint64_t readPos, uint64_t readSize) {
		for(uint64_t i = 0; i < Disk::SectorSize; i += VSA::StandardRecordSize) {
			std::shared_ptr<VSA> testRecord;
			const auto& testRecordStatus = parser.getRecordFromBytes(buffer.data() + i, static_cast<size_t>(readSize - i), testRecord);
			if(testRecordStatus == VSAParser::RecordParseStatus::Success) {
				const auto& testTimestamp = testRecord->getTimestamp();
				uint64_t tickDiff = (testTimestamp > desiredTimestamp) 
					? testTimestamp - desiredTimestamp 
					: desiredTimestamp - testTimestamp;
				if(tickDiff < bestTickDiff) {
					bestTickDiff = tickDiff;
					bestOffset = readPos + i;
					bestRecord = testRecord;
				}
			}
		}
	};

	if(firstOffset % Disk::SectorSize != 0) {
		// First record is not located at the beginning of a sector
		// Find the best tick diff for the sector that the first record is in
		// and ignore it during the binary search to allow for binary searching sectors, not bytes.
		// Compare the bestTickDiff from the firstSector to results during the linear search to 
		// find the true best tick diff.
		uint64_t readPos = firstOffset - (firstOffset % Disk::SectorSize);
		const auto& bytesRead = vsaReadLogicalDisk(readPos, buffer.data(), Disk::SectorSize, metadata);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}

		findBestTickDiff(readPos, Disk::SectorSize);
		
		// Change the first offset to eliminate the first sector from the binary search
		firstOffset = firstOffset + Disk::SectorSize - (firstOffset % Disk::SectorSize);
	}

	uint64_t leftIndex = 0; // Index of the leftmost sector (not the byte)
	uint64_t rightIndex = ((metadata.isOverlapped) ? metadata.diskSize : metadata.bufferEnd) - VSA::RecordStartOffset;
	rightIndex = (rightIndex / Disk::SectorSize);
	uint64_t midIndex; // Index of the middle sector (not the byte)
	// Indices indicate sector offset from first sector location in metadata
	while(leftIndex < rightIndex) { // Perform binary search for records with timestamp closest to the desired timestamp
		midIndex = (rightIndex + leftIndex) / 2;
		uint64_t readPos = firstOffset + midIndex * Disk::SectorSize;
		const auto& bytesRead = vsaReadLogicalDisk(readPos, buffer.data(), Disk::SectorSize, metadata);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		std::shared_ptr<VSA> midRecord;
		auto midRecordStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, midRecord);
		switch(midRecordStatus) {
			case VSAParser::RecordParseStatus::NotARecordStart:
				// This part of the buffer does not contain records
				rightIndex = midIndex - 1;
				continue;
			case VSAParser::RecordParseStatus::ConsecutiveExtended:
				// We dropped in the middle of an extended message record
				auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(midRecord); 
				uint64_t pos = readPos;
				if(!findFirstExtendedVSAFromConsecutive(extendedRecord, pos, parser, metadata)) {
					pos = firstOffset + midIndex * Disk::SectorSize;
					if(!findPreviousRecordWithTimestamp(midRecord, pos, parser)) {
						return false;
					}
					midIndex = (pos - firstOffset) / Disk::SectorSize;
					break;
				}
				midIndex = (pos - firstOffset) / Disk::SectorSize;
				midRecord = extendedRecord;
				break;
		}
		if(midIndex <= leftIndex) {
			// Extended records cause problems with binary search
			// Just move on to linear search
			leftIndex = midIndex;
			break;
		} 
		if(midIndex > rightIndex) {
			// Extended records cause problems with binary search
			// Just move on to linear search
			rightIndex = midIndex;
			break;
		}
		if(!midRecord || !midRecord->isTimestampValid()) { // Unhandled failure to get timestamp
			report(APIEvent::Type::VSATimestampNotFound, APIEvent::Severity::Error);
			return false;
		}
		if(midRecord->getTimestamp() == desiredTimestamp) {
			vsaOffset = firstOffset + midIndex * Disk::SectorSize;
			record = midRecord;
			return true;
		}
		if(midRecord->getTimestamp() < desiredTimestamp && midRecord->getTimestamp() > desiredTimestamp - LinearSearchTickDifference) {
			// The timestamp of this sector is within desired linear search range
			const uint64_t LinearReadByteAmount = 0x1000ull; // Number of bytes to read for linear search
			uint64_t pos = firstOffset + midIndex * Disk::SectorSize;
			const auto& recordBufferSize = metadata.diskSize - VSA::RecordStartOffset;
			pos -= ((pos - VSA::RecordStartOffset) / recordBufferSize) * recordBufferSize; // Move pos to within physical space of record buffer
			uint64_t lastTickDiff = UINT64_MAX;
			buffer.resize(LinearReadByteAmount);

			// Begin linear search for record with closest timestamp to desired
			while(lastTickDiff < LinearSearchTickDifference || lastTickDiff == UINT64_MAX) {
				const auto& bytesReadLinear = vsaReadLogicalDisk(pos, buffer.data(), LinearReadByteAmount, metadata);
				if(!bytesReadLinear || *bytesReadLinear < LinearReadByteAmount) {
					report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
					return false;
				}
				for(uint64_t i = 0; i < LinearReadByteAmount; i += VSA::StandardRecordSize) {
					std::shared_ptr<VSA> testRecord;
					const auto& testRecordStatus = parser.getRecordFromBytes(buffer.data() + i, static_cast<size_t>(LinearReadByteAmount - i), testRecord);
					if(testRecordStatus == VSAParser::RecordParseStatus::Success) {
						const auto& testTimestamp = testRecord->getTimestamp();
						uint64_t tickDiff = (testTimestamp > desiredTimestamp)
												? testTimestamp - desiredTimestamp
												: desiredTimestamp - testTimestamp;
						if(tickDiff < bestTickDiff) {
							bestTickDiff = tickDiff;
							bestRecord = testRecord;
							bestOffset = pos + i;
						}
						lastTickDiff = tickDiff;
						if(testRecord->getType() == VSA::Type::AA6A) {
							// Increment to skip the whole record if it is a full sector length record
							i += Disk::SectorSize - VSA::StandardRecordSize;
						} 
					} else if(testRecordStatus == VSAParser::RecordParseStatus::NotARecordStart) {
						// We have reached an area with no records
						break;
					} else {
						lastTickDiff = UINT64_MAX;
					}
				}
				pos += LinearReadByteAmount;
				if(!metadata.isOverlapped && pos > metadata.bufferEnd) {
					// Ran out of valid records to search
					break;
				}
				if(metadata.isOverlapped && pos >= metadata.firstRecordLocation && 
					pos - LinearReadByteAmount < metadata.firstRecordLocation) {
					// Ran out of valid records to search
					break;
				}
			}
			// Return the location with the closest timestamp to the desired point
			vsaOffset = bestOffset;
			record = bestRecord;
			return true;
		} else if(midRecord->getTimestamp() < desiredTimestamp) {
			// Keep right partition
			leftIndex = midIndex;
		} else {
			// Keep left partition
			rightIndex = midIndex;
		}
	}
	
	// Nothing found within desired linear search tick range
	// Check for closest record to desired timestamp
	if(leftIndex <= rightIndex) {
		buffer.clear();
		uint64_t pos = firstOffset + leftIndex * Disk::SectorSize;
		uint64_t bufferSize = (rightIndex - leftIndex + 1) * Disk::SectorSize;
		buffer.resize(static_cast<size_t>(bufferSize));
		const auto& bytesRead = vsaReadLogicalDisk(pos, buffer.data(), Disk::SectorSize, metadata);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return false;
		}
		findBestTickDiff(pos, bufferSize);
		vsaOffset = bestOffset;
		record = bestRecord;
		return true;
	}
	report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
	return false;
}

std::optional<uint64_t> Device::getVSATimestampOrBefore(VSAParser& parser, std::vector<uint8_t>& buffer, uint64_t pos, uint64_t minPos,
	std::optional<VSAMetadata> optMetadata) {
	VSAMetadata metadata;
	if(!optMetadata) {
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return std::nullopt;
		}
		metadata.diskSize = *diskSize;
	} else {
		metadata = *optMetadata;
	}
	
	while(pos >= minPos) {
		const auto& bytesRead = readLogicalDisk(pos, buffer.data(), Disk::SectorSize);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return std::nullopt;
		}
		std::shared_ptr<VSA> record;
		auto parseStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, record);
		// Handle situations where we were dropped in the middle of an extended record
		if(parseStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
			auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(record);
			auto tempPos = pos; // If the first-extended search fails, we still have the original value of the search start location
			if(!findFirstExtendedVSAFromConsecutive(extendedRecord, tempPos, parser, metadata)) {
				// Unoptimized/Unloopable search
				if(!findPreviousRecordWithTimestamp(record, pos, parser)) {
					return std::nullopt;
				}
				return record->getTimestamp();
			}
			return extendedRecord->getTimestamp();
		}
		// Handle other situations without valid timestamp
		if(parseStatus != VSAParser::RecordParseStatus::Success) {
			pos -= VSA::StandardRecordSize;
		} else {
			return record->getTimestamp();
		}
	}
	report(APIEvent::Type::VSATimestampNotFound, APIEvent::Severity::Error);
	return std::nullopt;
}

std::optional<uint64_t> Device::getVSATimestampOrAfter(VSAParser& parser, std::vector<uint8_t>& buffer, uint64_t pos, uint64_t maxPos,
	std::optional<VSAMetadata> optMetadata) {
	VSAMetadata metadata;
	if(!optMetadata) {
		const auto& diskSize = getVSADiskSize();
		if(!diskSize) {
			return std::nullopt;
		}
		metadata.diskSize = *diskSize;
	} else {
		metadata = *optMetadata;
	}
	
	while(pos <= maxPos) {
		const auto& bytesRead = readLogicalDisk(pos, buffer.data(), Disk::SectorSize);
		if(!bytesRead || *bytesRead < Disk::SectorSize) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			return std::nullopt;
		}
		std::shared_ptr<VSA> record;
		auto parseStatus = parser.getRecordFromBytes(buffer.data(), Disk::SectorSize, record);
		if(parseStatus == VSAParser::RecordParseStatus::ConsecutiveExtended) {
			auto extendedRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(record);
			auto tempPos = pos; // If the first-extended search fails, we still have the original value of the search start location
			if(!findFirstExtendedVSAFromConsecutive(extendedRecord, tempPos, parser, metadata)) {
				pos += VSA::StandardRecordSize;
				continue;
			}
			return extendedRecord->getTimestamp();
		}
		// Other situations with invalid timestamp
		if(parseStatus != VSAParser::RecordParseStatus::Success) {
			pos += VSA::StandardRecordSize;
		} else {
			return record->getTimestamp();
		}
	}
	report(APIEvent::Type::VSATimestampNotFound, APIEvent::Severity::Error);
	return std::nullopt;
}

bool Device::parseVSA(
	VSAMetadata& metadata, const VSAExtractionSettings& extractionSettings, const VSAMessageReadFilter& filter
) {
	static constexpr uint64_t MaxReadAmountPerIteration = 0x10000; // Read 512 KB per iteration
	static constexpr uint64_t MaxReadFailuresAllowed = 10;

	if(filter.readRange.first > filter.readRange.second) {
		// First timestamp occurs after second timestamp in filter
		// Do not fail to allow for other filters to do work
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::EventWarning);
		return true; 
	}

	// Location of first time_point from the filter
	uint64_t readOffset;
	std::shared_ptr<VSA> recordAtOffset;
	if(!findVSAOffsetFromTimepoint(filter.readRange.first, readOffset, recordAtOffset, extractionSettings, metadata)) {
		report(APIEvent::Type::VSATimestampNotFound, APIEvent::Severity::Error);
		return false;
	}
	if(readOffset >= metadata.diskSize) {
		readOffset -= metadata.diskSize - VSA::RecordStartOffset;
	}

	std::vector<uint8_t> buffer;
	bool success = true;
	bool moreToRead = true;
	VSAParser::Settings parserSettings = VSAParser::Settings::messageRecords();
	VSAParser parser(report, parserSettings);
	parser.setMessageFilter(filter);
	while(moreToRead) {
		uint64_t amount;
		if(!metadata.isOverlapped) {
			moreToRead = readOffset + MaxReadAmountPerIteration < metadata.bufferEnd;
			amount = std::min(MaxReadAmountPerIteration, metadata.bufferEnd - readOffset);
		} else {
			uint64_t readEnd = readOffset + MaxReadAmountPerIteration;
			if(readEnd > metadata.diskSize) {
				// Make sure read end is within the buffer for a possible looped read
				readEnd -= metadata.diskSize - VSA::RecordStartOffset;
			}
			moreToRead = !(readOffset < metadata.bufferEnd && readEnd >= metadata.bufferEnd);
			amount = moreToRead ? MaxReadAmountPerIteration : metadata.bufferEnd - readOffset;
		}
		if(amount < VSA::StandardRecordSize) {
			break;
		}

		buffer.resize(static_cast<size_t>(amount));
		uint64_t readAttempt = 1;
		while(true) {
			const auto& bytesRead = vsaReadLogicalDisk(readOffset, buffer.data(), amount, metadata);
			if(bytesRead && *bytesRead == amount) {
				break;
			}
			if(readAttempt >= MaxReadFailuresAllowed) {
				report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
				return false;
			}
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::EventWarning);
			++readAttempt;
		}

		success = parser.parseBytes(buffer.data(), amount);
		if(!success) {
			report(APIEvent::Type::VSAByteParseFailure, APIEvent::Severity::Error);
			return false;
		}
		if(parser.empty()) { // No message records or all extended message records were not terminated in block
			readOffset += amount;
			continue;
		}
		const auto& lastRecord = parser.back();
		if(lastRecord->getTimestampICSClock() >= filter.readRange.second) {
			// Reached the desired end timestamp
			moreToRead = false;
		}
		if(!dispatchVSAMessages(parser)) { // Piecemeal dispatch messages so we don't have exceptionally large vectors
			return false;
		}
		readOffset += amount;
	}
	return success;
}

std::optional<uint64_t> Device::vsaReadLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount, std::optional<VSAMetadata> metadata) {
	uint64_t diskSize;
	if(metadata) {
		diskSize = metadata->diskSize;
	} else {
		const auto& testDiskSize = getVSADiskSize();
		if(!testDiskSize) {
			return std::nullopt;
		}
		diskSize = *testDiskSize;
	}
	// Set the position to be within the ring buffer
	if(pos < VSA::RecordStartOffset) {
		pos = diskSize - (VSA::RecordStartOffset - pos);
	} else if(pos >= diskSize) {
		const auto& bufferSize = diskSize - VSA::RecordStartOffset;
		const auto& timesLooped = (pos - VSA::RecordStartOffset) / bufferSize;
		pos -= bufferSize * timesLooped;
	}

	if(amount > diskSize - VSA::RecordStartOffset) { // Given read amount is too large
		amount = diskSize - VSA::RecordStartOffset; // Do full disk dump
	}
	if(pos + amount < diskSize) { // Read doesn't need to loop
		return readLogicalDisk(pos, into, amount);
	}
	uint64_t firstReadAmount = diskSize - pos;
	uint64_t bytesRead = 0;
	if(auto optBytesReadFirst = readLogicalDisk(pos, into, firstReadAmount)) {
		bytesRead = *optBytesReadFirst;
	} else {
		return std::nullopt;
	}
	if(auto optBytesReadSecond = readLogicalDisk(VSA::RecordStartOffset, into + firstReadAmount, amount - firstReadAmount)) {
		bytesRead += *optBytesReadSecond;
	} else {
		return std::nullopt;
	}
	return bytesRead;
}

bool Device::dispatchVSAMessages(VSAParser& parser) {
	std::vector<std::shared_ptr<Packet>> packets;
	if(!parser.extractMessagePackets(packets)) {
		report(APIEvent::Type::VSAByteParseFailure, APIEvent::Severity::Error);
		return false;
	}
	for(const auto& packet : packets) {
		std::shared_ptr<Message> msg;
		if(!com->decoder->decode(msg, packet)) {
			return false;
		}
		com->dispatchMessage(msg);
	}
	return true;
}

std::optional<uint64_t> Device::getCoreMiniScriptTimestamp() {
	static constexpr auto CoreMiniTimestampLocation = 48;
	static constexpr auto CoreMiniTimestampSize = 8;
	uint8_t buffer[CoreMiniTimestampSize];
	const auto& numBytes = readLogicalDisk(CoreMiniTimestampLocation, buffer, CoreMiniTimestampSize);
	if(!numBytes || *numBytes < CoreMiniTimestampSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return std::nullopt;
	}
	return *reinterpret_cast<uint64_t*>(buffer);
}

std::optional<uint64_t> Device::getVSADiskSize() {
	uint64_t diskSize;
	auto scriptStatus = getScriptStatus();
	if(!scriptStatus) {
		return std::nullopt;
	}
	if(!scriptStatus->isCoreminiRunning) {
		startScript();
		scriptStatus = getScriptStatus();
		if(!scriptStatus) {
			return std::nullopt;
		}
		diskSize = (scriptStatus->maxSector + 1) * Disk::SectorSize;
		stopScript();
	} else {
		diskSize = (scriptStatus->maxSector + 1) * Disk::SectorSize;
	}
	if(diskSize == Disk::SectorSize) {
		report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
		return std::nullopt;
	}
	return diskSize;
}

bool Device::requestTC10Wake(Network::NetID network) {
	if(!supportsTC10()) {
		report(APIEvent::Type::NotSupported, APIEvent::Severity::Error);
		return false;
	}
	std::vector<uint8_t> args(sizeof(network));
	*(Network::NetID*)args.data() = network;
	auto msg = com->waitForMessageSync([&] {
		return com->sendCommand(ExtendedCommand::RequestTC10Wake, args);
	}, std::make_shared<MessageFilter>(Message::Type::ExtendedResponse), std::chrono::milliseconds(1000));

	if(!msg) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	auto resp = std::static_pointer_cast<ExtendedResponseMessage>(msg);
	if(!resp) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return false;
	}

	return resp->response == ExtendedResponse::OK;
}

bool Device::requestTC10Sleep(Network::NetID network) {
	if(!supportsTC10()) {
		report(APIEvent::Type::NotSupported, APIEvent::Severity::Error);
		return false;
	}
	std::vector<uint8_t> args(sizeof(network));
	*(Network::NetID*)args.data() = network;
	auto msg = com->waitForMessageSync([&] {
		return com->sendCommand(ExtendedCommand::RequestTC10Sleep, args);
	}, std::make_shared<MessageFilter>(Message::Type::ExtendedResponse), std::chrono::milliseconds(1000));

	if(!msg) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	auto typed = std::static_pointer_cast<ExtendedResponseMessage>(msg);
	if(!typed) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return false;
	}

	return typed->response == ExtendedResponse::OK;
}

std::optional<TC10StatusMessage> Device::getTC10Status(Network::NetID network) {
	if(!supportsTC10()) {
		report(APIEvent::Type::NotSupported, APIEvent::Severity::Error);
		return std::nullopt;
	}
	std::vector<uint8_t> args(sizeof(network));
	*(Network::NetID*)args.data() = network;
	auto msg = com->waitForMessageSync([&] {
		return com->sendCommand(ExtendedCommand::GetTC10Status, args);
	}, std::make_shared<MessageFilter>(Message::Type::TC10Status), std::chrono::milliseconds(1000));

	if(!msg) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	auto typed = std::static_pointer_cast<TC10StatusMessage>(msg);
	
	if(!typed) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return *typed;
}

std::optional<GPTPStatus> Device::getGPTPStatus(std::chrono::milliseconds timeout) {
	if(!supportsGPTP()) {
		report(APIEvent::Type::GPTPNotSupported, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this](){ 
			return com->sendCommand(ExtendedCommand::GetGPTPStatus, {}); 
		},
		std::make_shared<MessageFilter>(Message::Type::GPTPStatus),
		timeout
	);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<GPTPStatus>(response);
	if(!retMsg) {
		return std::nullopt;
	}

	return *retMsg;
}

bool Device::writeMACsecConfig(const MACsecMessage& message, uint16_t binaryIndex)
{
	std::vector<uint8_t> raw;

	message.EncodeFromMessage(raw, report);

	return writeBinaryFile(raw, binaryIndex);
}

bool Device::enableNetworkCommunication(bool enable, uint32_t timeout) {
	bool sendMsg = false;
	if(!com->driver->enableCommunication(enable, sendMsg)) {
		return false;
	}
	if(sendMsg) {
		const uint8_t* i = (uint8_t*)&timeout;
		if(!com->sendCommand(Command::EnableNetworkCommunication, {enable, 0, 0, 0, i[0], i[1], i[2], i[3]})) {
			return false;
		}
	}
	return true;
}



bool Device::formatDisk(const DiskDetails& config, const DiskFormatProgress& handler, std::chrono::milliseconds interval) {
#pragma pack(push, 2)
	struct DiskFormatProgressResponse {
		uint16_t state;
		uint64_t sectorsRemaining;
	};
#pragma pack(pop)

	auto diskCount = getDiskCount();
	
	if(!diskCount) {
		report(APIEvent::Type::DiskFormatNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(config.disks.size() != diskCount) {
		report(APIEvent::Type::DiskFormatInvalidCount, APIEvent::Severity::Error);
		return false;
	}

	std::vector<uint8_t> payload = DiskDetails::Encode(config);
	if(!com->sendCommand(ExtendedCommand::DiskFormatStart, payload)) {
		return false;
	}

	uint64_t sectorsFormatted = 0;
	uint64_t sectorsTotal = 0;
	uint16_t lastState = 1;
	do {
		std::shared_ptr<Message> response = com->waitForMessageSync(
			[this](){ 
				return com->sendCommand(ExtendedCommand::DiskFormatProgress, {}); 
			},
			std::make_shared<ExtendedResponseFilter>(ExtendedCommand::DiskFormatProgress),
			std::chrono::milliseconds(200)
		);

		if(!response) {
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
			return false;
		}

		auto extResponse = std::dynamic_pointer_cast<ExtendedResponseMessage>(response);
		if(!extResponse) {
			// Should never happen
			return false;
		}
		
		if(extResponse->data.size() < sizeof(DiskFormatProgressResponse)) {
			report(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error);
			return false;
		}

		auto* progress = reinterpret_cast<DiskFormatProgressResponse*>(extResponse->data.data());
		lastState = progress->state;

		if(sectorsTotal == 0) {
			sectorsTotal = progress->sectorsRemaining;
		} else {
			sectorsFormatted = sectorsTotal - progress->sectorsRemaining;

			if(handler) {
				auto directive = handler(sectorsFormatted, sectorsTotal);

				if(directive == DiskFormatDirective::Stop) {
					return com->sendCommand(ExtendedCommand::DiskFormatCancel);
				}
			}
		}

		std::this_thread::sleep_for(interval);
	} while(lastState);

	return true;
}

bool Device::forceDiskConfigUpdate(const DiskDetails& config) {
	auto diskCount = getDiskCount();
	
	if(!diskCount) {
		report(APIEvent::Type::DiskFormatNotSupported, APIEvent::Severity::Error);
		return false;
	}
	if(config.disks.size() != diskCount) {
		report(APIEvent::Type::DiskFormatInvalidCount, APIEvent::Severity::Error);
		return false;
	}

	return com->sendCommand(ExtendedCommand::DiskFormatUpdate, DiskDetails::Encode(config));
}

std::shared_ptr<DiskDetails> Device::getDiskDetails(std::chrono::milliseconds timeout) {
	if(!supportsDiskFormatting()) {
		report(APIEvent::Type::DiskFormatNotSupported, APIEvent::Severity::Error);
		return nullptr;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullptr;
	}

	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this](){ 
			return com->sendCommand(ExtendedCommand::GetDiskDetails, {}); 
		},
		std::make_shared<ExtendedResponseFilter>(ExtendedCommand::GetDiskDetails),
		timeout
	);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return nullptr;
	}

	auto extResponse = std::dynamic_pointer_cast<ExtendedResponseMessage>(response);
	if(!extResponse) {
		// Should never happen
		return nullptr;
	}

	return DiskDetails::Decode(extResponse->data, getDiskCount(), report);
}

