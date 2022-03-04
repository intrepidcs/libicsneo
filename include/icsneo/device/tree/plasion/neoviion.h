#ifndef __NEOVIION_H_
#define __NEOVIION_H_

#ifdef __cplusplus

#include "icsneo/device/tree/plasion/plasion.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/disk/plasiondiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"

namespace icsneo {

class NeoVIION : public Plasion {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::ION;
	static constexpr const uint16_t PRODUCT_ID = 0x0901;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoVIION(neodevice));

		return found;
	}

private:
	NeoVIION(neodevice_t neodevice) : Plasion(neodevice) {
		initialize<FTDI, NullSettings, Disk::PlasionDiskReadDriver, Disk::NeoMemoryDiskDriver>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

	virtual std::shared_ptr<Communication> makeCommunication(
		std::unique_ptr<Driver> transport,
		std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer,
		std::unique_ptr<Encoder> encoder,
		std::unique_ptr<Decoder> decoder
	) override {
		return std::make_shared<MultiChannelCommunication>(
			report,
			std::move(transport),
			makeConfiguredPacketizer,
			std::move(encoder),
			std::move(decoder),
			1 // 2
		);
	}
};

}

#endif // __cplusplus

#endif