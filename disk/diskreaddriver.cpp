#include "icsneo/disk/diskreaddriver.h"
#include <cstring>

using namespace icsneo;

optional<uint64_t> DiskReadDriver::readLogicalDisk(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	optional<uint64_t> ret;

	// Read into here if we can't read directly into the user buffer
	// That would be the case either if we don't want some at the
	// beginning or end of the block.
	std::vector<uint8_t> alignedReadBuffer;

	const uint32_t idealBlockSize = getBlockSizeBounds().second;
	const uint64_t startBlock = pos / idealBlockSize;
	const uint64_t posWithinFirstBlock = pos % idealBlockSize;
	const uint64_t blocks = amount / idealBlockSize + (amount % idealBlockSize ? 1 : 0) + (posWithinFirstBlock ? 1 : 0);
	uint64_t blocksProcessed = 0;

	while(blocksProcessed < blocks && timeout >= std::chrono::milliseconds::zero()) {
		const uint64_t currentBlock = startBlock + blocksProcessed;

		const int intoOffset = std::min<int>((blocksProcessed * idealBlockSize) - posWithinFirstBlock, 0);
		const auto posWithinCurrentBlock = (blocksProcessed ? 0 : posWithinFirstBlock);
		auto curAmt = idealBlockSize - posWithinCurrentBlock;
		const auto amountLeft = amount - *ret;
		if(curAmt > amountLeft)
			curAmt = amountLeft;

		const bool useAlignedReadBuffer = (posWithinCurrentBlock != 0 || curAmt != idealBlockSize);
		if(useAlignedReadBuffer && alignedReadBuffer.size() < idealBlockSize)
			alignedReadBuffer.resize(idealBlockSize);

		auto start = std::chrono::high_resolution_clock::now();
		auto readAmount = readLogicalDiskAligned(com, report, currentBlock * idealBlockSize,
			useAlignedReadBuffer ? alignedReadBuffer.data() : into + intoOffset, idealBlockSize, timeout);
		timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

		if(!readAmount.has_value() || *readAmount == 0) {
			if(timeout < std::chrono::milliseconds::zero())
				report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
			else
				report(blocksProcessed ? APIEvent::Type::EOFReached : APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			break;
		}

		if(useAlignedReadBuffer)
			memcpy(into + intoOffset, alignedReadBuffer.data() + posWithinCurrentBlock, curAmt);

		if(!ret)
			ret.emplace();
		*ret += *readAmount;
		blocksProcessed++;
	}

	return ret;
}