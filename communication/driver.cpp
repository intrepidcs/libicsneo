#include "icsneo/communication/driver.h"

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

	return actuallyRead > 0;
}

bool Driver::write(const std::vector<uint8_t>& bytes) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(writeBlocks) {
		if(writeQueue.size_approx() > writeQueueSize)
			while(writeQueue.size_approx() > (writeQueueSize * 3 / 4))
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} else {
		if(writeQueue.size_approx() > writeQueueSize) {
			report(APIEvent::Type::TransmitBufferFull, APIEvent::Severity::Error);
			return false;
		}
	}

	bool ret = writeQueue.enqueue(WriteOperation(bytes));
	if(!ret)
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);

	return ret;
}