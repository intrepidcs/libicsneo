#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/api/lifetime.h"
#include <cstring>

#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
#include <iostream>
#endif

using namespace icsneo;
using namespace icsneo::Disk;

optional<uint64_t> ExtExtractorDiskReadDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	static std::shared_ptr<MessageFilter> NeoMemorySDRead = std::make_shared<MessageFilter>(Network::NetID::NeoMemorySDRead);

	if(amount > getBlockSizeBounds().second)
		return nullopt;

	if(amount % getBlockSizeBounds().first != 0)
		return nullopt;

	if(pos % getBlockSizeBounds().first != 0)
		return nullopt;

	if(cachePos != pos || std::chrono::steady_clock::now() > cachedAt + CacheTime) {
		uint64_t sector = pos / SectorSize;

		uint64_t largeSectorCount = amount / SectorSize;
		uint32_t sectorCount = uint32_t(largeSectorCount);
		if (largeSectorCount != uint64_t(sectorCount))
			return nullopt;

		// The cache does not have this data, go get it
		std::mutex m;
		std::condition_variable cv;
		uint16_t receiving = 0; // How much are we about to get before another header or completion
		uint64_t received = 0;
		uint16_t receivedCurrent = 0;
		uint8_t gotHeaderBytes = 0;
		std::unique_lock<std::mutex> lk(m);
		bool error = !com.redirectRead([&](std::vector<uint8_t>&& data) {
			std::unique_lock<std::mutex> lk2(m);
			size_t offset = 0;
			while(offset < data.size()) {
				size_t left = data.size() - offset;
				if(gotHeaderBytes != headerLength) {
					if(gotHeaderBytes == 0 && left && data[offset] != 0xaa) {
						#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
						std::cout << "Incorrect header " << int(data[offset]) << ' ' << int(offset) << std::endl;
						#endif
						offset++;
						continue;
					}

					// Did we get a correct header and at least one byte of data?
					if(int32_t(left) < (headerLength - gotHeaderBytes)) {
						#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
						std::cout << "Got " << int(left) << " bytes of header at " << offset << std::endl;
						#endif
						gotHeaderBytes += uint8_t(left);
						return;
					}

					// The device tells us how much it's sending us before the next header
					receiving = (data[offset + headerLength-2-gotHeaderBytes] | (data[offset + headerLength-1-gotHeaderBytes] << 8));
					#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
					std::cout << "Started packet of size " << receiving << " bytes" << std::endl;
					#endif

					// Skip the header and any bytes necessary for unaligned read
					offset += headerLength - gotHeaderBytes;
					gotHeaderBytes = headerLength;
				}

				const auto available = left - offset;
				auto count = uint16_t(std::min<uint64_t>(std::min<uint64_t>(receiving - receivedCurrent, available), amount - received));
				memcpy(cache.data() + received, data.data() + offset, count);
				received += count;
				receivedCurrent += count;
				offset += count;

				if(amount == received) {
					if(receivedCurrent % 2 == 0)
						offset++;
					lk2.unlock();
					cv.notify_all();
					lk2.lock();
					#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
					std::cout << "Finished!" << std::endl;
					#endif
				}
				else if(receivedCurrent == receiving) {
					#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
					std::cout << "Got " << count << " bytes, " << receivedCurrent << " byte packet " << received <<
						" complete of " << amount << std::endl;
					#endif
					if(receivedCurrent % 2 == 0)
						offset++;
					gotHeaderBytes = 0; // Now we will need another header
					receivedCurrent = 0;
				} else {
					#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
					std::cout << "Got " << count << " bytes, incomplete (of " << receiving << " bytes)" << std::endl;
					#endif
				}
			}
		});
		Lifetime clearRedirect([&com, &lk] { lk.unlock(); com.clearRedirectRead(); });

		if(error)
			return nullopt;

		error = !com.sendCommand(ExtendedCommand::Extract, {
			uint8_t(sector & 0xff),
			uint8_t((sector >> 8) & 0xff),
			uint8_t((sector >> 16) & 0xff),
			uint8_t((sector >> 24) & 0xff),
			uint8_t((sector >> 32) & 0xff),
			uint8_t((sector >> 40) & 0xff),
			uint8_t((sector >> 48) & 0xff),
			uint8_t((sector >> 56) & 0xff),
			uint8_t(sectorCount & 0xff),
			uint8_t((sectorCount >> 8) & 0xff),
			uint8_t((sectorCount >> 16) & 0xff),
			uint8_t((sectorCount >> 24) & 0xff),
		});
		if(error)
			return nullopt;

		bool hitTimeout = !cv.wait_for(lk, timeout, [&]() { return error || amount == received; });
		if(hitTimeout || error)
			return nullopt;

		cachedAt = std::chrono::steady_clock::now();
		cachePos = pos;
	}

	memcpy(into, cache.data(), size_t(amount));
	return amount;
}