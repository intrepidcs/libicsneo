#include "icsneo/core/ringbuffer.h"
#include <stdexcept>

namespace icsneo {
	
RingBuffer::RingBuffer(size_t bufferSize) : readCursor(0), writeCursor(0) {
	// round the buffer size to the nearest power of 2
	bufferSize = RoundUp(bufferSize);
	mask = bufferSize - 1;
	buf = new uint8_t[bufferSize];
}

RingBuffer::~RingBuffer() {
	delete[] buf;
	buf = nullptr;
}

const uint8_t& RingBuffer::operator[](size_t offset) const { 
	return get(offset); 
}

size_t RingBuffer::size() const { 
	// The values in the cursors are monotonic, i.e. they only ever increment. They can be considered to be the total number of elements ever written or read
	auto currentWriteCursor = writeCursor.load(std::memory_order_relaxed);
	auto currentReadCursor = readCursor.load(std::memory_order_relaxed);
	// Using unmasked values, writeCursor is guaranteed to be >= readCursor. If they are equal that means the buffer is empty
	return currentWriteCursor - currentReadCursor; 
}

void RingBuffer::pop_front() {
	pop(1);
}

void RingBuffer::pop(size_t count) {
	if (size() < count) {
		throw std::runtime_error("RingBuffer: Underflow");
	}
	readCursor.fetch_add(count, std::memory_order_release);
}

const uint8_t& RingBuffer::get(size_t offset) const {
	if (offset >= size()) {
		throw std::runtime_error("RingBuffer: Index out of range");
	}
	auto currentReadCursor = readCursor.load(std::memory_order_acquire);
	return *resolve(currentReadCursor, offset);
}

bool RingBuffer::write(const uint8_t* addr, size_t length) {    
	const auto freeSpace = (capacity() - size());
	if (length > freeSpace) {
		return false;
	}
	auto currentWriteCursor = writeCursor.load(std::memory_order_relaxed);
	auto spaceAtEnd = std::min(freeSpace, capacity() - (currentWriteCursor & mask)); // number of bytes from (masked) writeCursor to the end of the writable space (i.e. we reach the masked read cursor or the end of the buffer)
	auto firstCopySize = std::min(spaceAtEnd, length);
	(void)memcpy(resolve(currentWriteCursor, 0), addr, firstCopySize);
	if (firstCopySize < length)
	{
		(void)memcpy(buf, &addr[firstCopySize], length - firstCopySize);
	}

	writeCursor.store(currentWriteCursor + length, std::memory_order_release);
	return true;
}

bool RingBuffer::write(const std::vector<uint8_t>& source) {
	return write(source.data(), source.size());
}

bool RingBuffer::read(uint8_t* dest, size_t startIndex, size_t length) const {
	auto currentSize = size();
	if ((startIndex >= currentSize) || ((startIndex + length) > size())) {
		return false;
	}
	auto currentReadCursor = readCursor.load(std::memory_order_relaxed);
	auto bytesAtEnd = std::min<size_t>(capacity() - ((currentReadCursor + startIndex) & mask), length);
	const auto bytesAtStart = (length - bytesAtEnd);

	(void)memcpy(dest, resolve(currentReadCursor, startIndex), bytesAtEnd);
	if (bytesAtStart > 0) {
		(void)memcpy(&dest[bytesAtEnd], buf, bytesAtStart);
	}
	return true;
}

void RingBuffer::clear() {
	pop(size());
}

}