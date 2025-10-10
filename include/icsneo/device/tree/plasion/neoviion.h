#ifndef __NEOVIION_H_
#define __NEOVIION_H_

#ifdef __cplusplus

#include "icsneo/device/tree/plasion/plasion.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/plasiondiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"

namespace icsneo {

class NeoVIION : public Plasion {
public:
	// USB PID is 0x0901, standard driver is DXX
	ICSNEO_FINDABLE_DEVICE(NeoVIION, DeviceType::ION, "40");

	ProductID getProductID() const override {
		return ProductID::neoVIION;
	}
private:
	NeoVIION(neodevice_t neodevice, const driver_factory_t& makeDriver) : Plasion(neodevice) {
		initialize<NullSettings, Disk::PlasionDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif // __cplusplus

#endif