#ifndef __PACKETIZER_H_
#define __PACKETIZER_H_

#ifdef __cplusplus

#include "icsneo/communication/packet.h"
#include "icsneo/api/eventmanager.h"
#include <queue>
#include <vector>
#include <memory>
#include <cstring>

#define ICSNEO_PACKETIZER_BUFFER_SIZE (512 * 1024)

namespace icsneo {

class Packetizer {
public:
	static uint8_t ICSChecksum(const std::vector<uint8_t>& data);

	Packetizer(device_eventhandler_t report) : report(report) {}

	std::vector<uint8_t>& packetWrap(std::vector<uint8_t>& data, bool shortFormat) const;

	bool input(const std::vector<uint8_t>& bytes);
	std::vector<std::shared_ptr<Packet>> output();

	bool disableChecksum = false; // Even for short packets
	bool align16bit = true; // Not needed for Mars, Galaxy, etc and newer
	
private:
	enum class ReadState {
		SearchForHeader,
		ParseHeader,
		ParseLongStylePacketHeader,
		ParseDiskDataHeader,
		GetData
	};

	class RingBuffer
	{
	private:
		constexpr static size_t mBufferSize = ICSNEO_PACKETIZER_BUFFER_SIZE;

		size_t mStartOffset;
		size_t mSize;
		uint8_t mData[mBufferSize];

	public:
		RingBuffer(void)
			: mStartOffset(0)
			, mSize(0)
		{
			(void)memset(mData, 0, mBufferSize);
		}

		const uint8_t& operator [](size_t offset) { return Get(offset); }
		size_t size(void) { return mSize; }
		void pop_front(void)
		{
			Erase_front(1);
		}

		void Erase_front(size_t count)
		{
			if (mSize < count)
			{
				throw std::runtime_error("RingBuffer: Underflow");
			}
			mStartOffset = (mStartOffset + count) % mBufferSize;
			mSize -= count;
		}

		const uint8_t& Get(size_t offset)
		{
			if (offset >= mSize)
			{
				throw std::runtime_error("RingBuffer: Index out of range");
			}
			return *Resolve(offset);
		}

		void Copy(const std::vector<uint8_t>& source)
		{
			const auto inputSize = source.size();
			const auto octetsAvailable = (mBufferSize - mSize);

			if (inputSize > octetsAvailable)
			{
				throw std::runtime_error("RingBuffer: Out of memory");
			}

			const auto octetsAvailableTail = (octetsAvailable - mStartOffset);
			const auto octetsToWrap = (inputSize > octetsAvailableTail) ? (inputSize - octetsAvailableTail) : 0;
			const auto octetsToAppend = (inputSize - octetsToWrap);

			(void)memcpy(Resolve(mSize), source.data(), octetsToAppend);
			if (octetsToWrap > 0)
			{
				(void)memcpy(mData, &source.data()[octetsToAppend], octetsToWrap);
			}
			mSize += inputSize;
		}

		void CopyTo(uint8_t* dest, size_t startIndex, size_t length)
		{
			if ((startIndex + length) > mSize)
			{
				throw std::runtime_error("RingBuffer: Index out of range");
			}

			const auto octetsToReadHead = std::min<size_t>((mBufferSize - mStartOffset - startIndex), length);
			const auto octetsToReadTail = (length - octetsToReadHead);

			(void)memcpy(dest, Resolve(startIndex), octetsToReadHead);
			if (octetsToReadTail > 0)
			{
				(void)memcpy(&dest[octetsToReadHead], mData, octetsToReadTail);
			}
		}

	protected:
		inline uint8_t* Resolve(size_t offset)
		{
			return &mData[(mStartOffset + offset) % mBufferSize];
		}
	};

	ReadState state = ReadState::SearchForHeader;

	int currentIndex = 0;
	int packetLength = 0;
	int headerSize = 0;
	bool checksum = false;
	bool gotGoodPackets = false; // Tracks whether we've ever gotten a good packet
	Packet packet;
	RingBuffer bytes;

	std::vector<std::shared_ptr<Packet>> processedPackets;

	device_eventhandler_t report;
};

}

#endif // __cplusplus

#endif