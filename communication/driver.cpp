#include "icsneo/communication/driver.h"

//#define ICSNEO_DRIVER_DEBUG_PRINTS
#ifdef ICSNEO_DRIVER_DEBUG_PRINTS
#include <iostream>
#include <iomanip>
#endif

using namespace icsneo;

bool Driver::readWait(std::vector<uint8_t>& bytes, std::chrono::milliseconds timeout, size_t limit) {
	// A limit of zero indicates no limit
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (readBuffer.size() + 4))
		limit = (readBuffer.size() + 4);

	bytes.resize(limit);

	// wait until we have enough data, or the timout occurs
	const auto timeoutTime = std::chrono::steady_clock::now() + timeout;
	while (readBuffer.size() < limit && std::chrono::steady_clock::now() < timeoutTime) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	size_t actuallyRead = std::min(readBuffer.size(), limit);
	readBuffer.read(bytes.data(), 0, actuallyRead);
	readBuffer.pop(actuallyRead);
	bytes.resize(actuallyRead);

#ifdef ICSNEO_DRIVER_DEBUG_PRINTS
	if(actuallyRead > 0) {
		std::cout << "Read data: (" << actuallyRead << ')' << std::hex << std::endl;
		for(int i = 0; i < actuallyRead; i += 16) {
			for(int j = 0; j < std::min<int>(actuallyRead - i, 16); j++)
				std::cout << std::setw(2) << std::setfill('0') << uint32_t(bytes[i+j]) << ' ';
			std::cout << std::endl;
		}
		std::cout << std::dec << std::endl;
	}
#endif

	return actuallyRead > 0;
}

bool Driver::write(const std::vector<uint8_t>& bytes) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(writeBlocks) {
		if(writeQueueFull()) {
			while(writeQueueAlmostFull()) // Wait until we have some decent amount of space
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	} else {
		if(writeQueueFull()) {
			report(APIEvent::Type::TransmitBufferFull, APIEvent::Severity::Error);
			return false;
		}
	}

	const bool ret = writeInternal(bytes);
	if(!ret)
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);

	return ret;
}