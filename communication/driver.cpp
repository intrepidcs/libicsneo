#include "icsneo/communication/driver.h"

//#define ICSNEO_DRIVER_DEBUG_PRINTS
#ifdef ICSNEO_DRIVER_DEBUG_PRINTS
#include <iostream>
#include <iomanip>
#endif

using namespace icsneo;

bool Driver::read(std::vector<uint8_t>& bytes, size_t limit) {
	// A limit of zero indicates no limit
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (readQueue.size_approx() + 4))
		limit = (readQueue.size_approx() + 4);

	if(bytes.capacity() < limit)
		bytes.resize(limit);

	size_t actuallyRead = readQueue.try_dequeue_bulk(bytes.data(), limit);

	if(bytes.size() > actuallyRead)
		bytes.resize(actuallyRead);

	return true;
}

bool Driver::readWait(std::vector<uint8_t>& bytes, std::chrono::milliseconds timeout, size_t limit) {
	// A limit of zero indicates no limit
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (readQueue.size_approx() + 4))
		limit = (readQueue.size_approx() + 4);

	bytes.resize(limit);

	size_t actuallyRead = readQueue.wait_dequeue_bulk_timed(bytes.data(), limit, timeout);

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