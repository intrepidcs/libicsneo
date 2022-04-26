#include "icsneo/disk/diskreaddriver.h"
#include <cstring>

using namespace icsneo;
using namespace icsneo::Disk;

optional<uint64_t> ReadDriver::readLogicalDisk(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	if(amount == 0)
		return 0;

	optional<uint64_t> ret;

	// Read into here if we can't read directly into the user buffer
	// That would be the case either if we don't want some at the
	// beginning or end of the block.
	std::vector<uint8_t> alignedReadBuffer;

	pos += vsaOffset;
	const uint32_t idealBlockSize = getBlockSizeBounds().second;
	const uint64_t startBlock = pos / idealBlockSize;
	const uint32_t posWithinFirstBlock = static_cast<uint32_t>(pos % idealBlockSize);
	uint64_t blocks = amount / idealBlockSize + (amount % idealBlockSize ? 1 : 0);
	if(blocks * idealBlockSize - posWithinFirstBlock < amount)
		blocks++; // We need one more block to get the last partial block's worth
	uint64_t blocksProcessed = 0;

	while(blocksProcessed < blocks && timeout >= std::chrono::milliseconds::zero()) {
		const uint64_t currentBlock = startBlock + blocksProcessed;

		uint64_t intoOffset = blocksProcessed * idealBlockSize;;
		if(intoOffset < posWithinFirstBlock)
			intoOffset = 0;
		else
			intoOffset -= posWithinFirstBlock;

		const uint32_t posWithinCurrentBlock = (blocksProcessed ? 0 : posWithinFirstBlock);
		uint32_t curAmt = idealBlockSize - posWithinCurrentBlock;
		const auto amountLeft = amount - ret.value_or(0);
		if(curAmt > amountLeft)
			curAmt = static_cast<uint32_t>(amountLeft);

		const bool useAlignedReadBuffer = (posWithinCurrentBlock != 0 || curAmt != idealBlockSize);
		if(useAlignedReadBuffer && alignedReadBuffer.size() < idealBlockSize)
			alignedReadBuffer.resize(idealBlockSize);

		auto start = std::chrono::high_resolution_clock::now();
		auto readAmount = readLogicalDiskAligned(com, report, currentBlock * idealBlockSize,
			useAlignedReadBuffer ? alignedReadBuffer.data() : (into + intoOffset), idealBlockSize, timeout);
		timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

		if(!readAmount.has_value() || *readAmount < curAmt) {
			if(timeout < std::chrono::milliseconds::zero())
				report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
			else
				report((blocksProcessed || readAmount.value_or(0u) != 0u) ? APIEvent::Type::EOFReached :
					APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			break;
		}

		if(useAlignedReadBuffer)
			memcpy(into + intoOffset, alignedReadBuffer.data() + posWithinCurrentBlock, curAmt);

		if(!ret)
			ret.emplace();
		*ret += std::min<uint64_t>(*readAmount, curAmt);
		blocksProcessed++;
	}

	return ret;
}