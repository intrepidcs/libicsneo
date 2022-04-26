#ifndef __NEOVIPLASMA_H_
#define __NEOVIPLASMA_H_

#ifdef __cplusplus

#include "icsneo/device/tree/plasion/plasion.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class NeoVIPLASMA : public Plasion {
public:
	// USB PID is 0x0801, standard driver is FTDI
	ICSNEO_FINDABLE_DEVICE_BY_PID(NeoVIPLASMA, DeviceType::PLASMA, 0x0801);

private:
	NeoVIPLASMA(neodevice_t neodevice, const driver_factory_t& makeDriver) : Plasion(neodevice) {
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
			1 // 3
		);
	}
};

}

#endif // __cplusplus

#endif