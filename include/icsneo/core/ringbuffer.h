#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <cstdint>
#include <cstddef>
#include <memory>
#include <cstring>
#include <mutex>
#include <atomic>
#include <vector>
#if __cplusplus >= 202002L
#include <bit>
#endif
namespace icsneo {

class RingBuffer
{
private:
	static constexpr size_t RoundUp(size_t size) {
		if (size == 0) {
			// Avoid underflow when decrementing later
			return 1;
		} else if (size >= SIZE_MAX) {
			// overflow case - resolve to max size
			return MaxSize;
		}
#if __cplusplus >= 202002L
		// c++20 gives us countl_zero which should be more effecient on most platforms
		auto lzero = std::countl_zero(size - 1);
		auto shift = (sizeof(size_t) * 8) - lzero;
		return 1ull << shift;
#else
		// Bit twiddling magic! See http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		--size;
		size |= size >> 1;
		size |= size >> 2;
		size |= size >> 4;
		for (size_t i = 1; i < sizeof(size_t); i <<= 1) {
			size |= size >> (i << 3);
		}
		++size;
		return size;
#endif
	}
	//static_assert(std::atomic<size_t>::is_always_lock_free, "RingBuffer cursor types are not lock-free");
	std::atomic<size_t> readCursor;
	std::atomic<size_t> writeCursor;
	// Use this to mask the cursor values to the buffer size. This is set to capacity - 1 where capacity is always an integral power of 2 (2, 4, 8, 16, etc)
	size_t mask;
	uint8_t* buf;

public:
	static constexpr auto MaxSize =  1ull << ((8 * sizeof(size_t)) - 1);
	RingBuffer(size_t bufferSize);
	~RingBuffer();
	const uint8_t& operator[](size_t offset) const;
	size_t size() const;
	void pop_front();
	void pop(size_t count);
	const uint8_t& get(size_t offset) const;
	bool write(const uint8_t* addr, size_t count);
	bool write(const std::vector<uint8_t>& source);
	bool read(uint8_t* dest, size_t startIndex, size_t length) const;
	void clear();
	constexpr size_t capacity() const {
		return mask + 1;
	}

protected:
	inline uint8_t* resolve(size_t cursor, size_t offset) const {
		return &buf[(cursor + offset) & mask];
	}
};
}
#endif