#ifndef __DRIVER_H_
#define __DRIVER_H_

#ifdef __cplusplus

#include <vector>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "icsneo/api/eventmanager.h"
#include "icsneo/third-party/concurrentqueue/blockingconcurrentqueue.h"
#include "icsneo/communication/ringbuffer.h"

namespace icsneo {

#define ICSNEO_DRIVER_RINGBUFFER_SIZE (512 * 1024)
class Driver {
public:
	Driver(const device_eventhandler_t& handler) : report(handler) {}
	virtual ~Driver() {}
	virtual bool open() = 0;
	virtual bool isOpen() = 0;
	virtual void modeChangeIncoming() {}
	virtual void awaitModeChangeComplete() {}
	virtual bool close() = 0;

	inline bool isDisconnected() const { return disconnected; };
	inline bool isClosing() const { return closing; }

	bool waitForRx(size_t limit, std::chrono::milliseconds timeout);
	bool waitForRx(std::function<bool()> predicate, std::chrono::milliseconds timeout);
	bool readWait(std::vector<uint8_t>& bytes, std::chrono::milliseconds timeout = std::chrono::milliseconds(100), size_t limit = 0);
	bool write(const std::vector<uint8_t>& bytes);
	virtual bool isEthernet() const { return false; }
	bool readAvailable() { return readBuffer.size() > 0; }
	RingBuffer& getReadBuffer() { return readBuffer; }

	virtual bool enableCommunication(bool /* enable */, bool& sendMsg) { sendMsg = true; return true; }

	device_eventhandler_t report;

	size_t writeQueueSize = 50;
	bool writeBlocks = true; // Otherwise it just fails when the queue is full

protected:
	class WriteOperation {
	public:
		WriteOperation() {}
		WriteOperation(const std::vector<uint8_t>& b) : bytes(b) {}
		std::vector<uint8_t> bytes;
	};
	enum IOTaskState {
		LAUNCH,
		WAIT
	};

	inline void setIsClosing(bool isClosing) { closing = isClosing; }
	inline void setIsDisconnected(bool isDisconnected) { disconnected = isDisconnected; }

	// Overridable in case the driver doesn't want to use writeTask and writeQueue
	virtual bool writeQueueFull() { return writeQueue.size_approx() > writeQueueSize; }
	virtual bool writeQueueAlmostFull() { return writeQueue.size_approx() > (writeQueueSize * 3 / 4); }
	virtual bool writeInternal(const std::vector<uint8_t>& b) { return writeQueue.enqueue(WriteOperation(b)); }

	bool pushRx(const uint8_t* buf, size_t numReceived);
	void clearBuffers();

	moodycamel::BlockingConcurrentQueue<WriteOperation> writeQueue;

private:
	RingBuffer readBuffer = RingBuffer(ICSNEO_DRIVER_RINGBUFFER_SIZE);
	std::condition_variable rxWaitCv;
	std::mutex rxWaitMutex;

	std::atomic<bool> closing{false};
	std::atomic<bool> disconnected{false};
};

}

#endif // __cplusplus

#endif