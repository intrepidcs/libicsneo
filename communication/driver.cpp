#include "icsneo/communication/driver.h"

//#define ICSNEO_DRIVER_DEBUG_PRINTS
#ifdef ICSNEO_DRIVER_DEBUG_PRINTS
#include <iostream>
#include <iomanip>
#endif

using namespace icsneo;

bool Driver::pushRx(const uint8_t* buf, size_t numReceived) {
	bool ret = readBuffer.write(buf, numReceived);

	rxWaitCv.notify_all();

	return ret;
}

void Driver::clearBuffers()
{
	WriteOperation flushop;

	readBuffer.clear();
	rxWaitCv.notify_all();

	while (writeQueue.try_dequeue(flushop)) {}
}

bool Driver::waitForRx(size_t limit, std::chrono::milliseconds timeout) {
	return waitForRx([limit, this]() {
		return readBuffer.size() >= limit;
	}, timeout);
}

bool Driver::waitForRx(std::function<bool()> predicate, std::chrono::milliseconds timeout) {
	std::unique_lock<std::mutex> lk(rxWaitMutex);
	return rxWaitCv.wait_for(lk, timeout, predicate);
}

bool Driver::readWait(std::vector<uint8_t>& bytes, std::chrono::milliseconds timeout, size_t limit) {
	// wait until we have enough data, or the timeout occurs
	waitForRx(limit, timeout);

	size_t actuallyRead = readBuffer.size();
	bytes.resize(actuallyRead);

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
