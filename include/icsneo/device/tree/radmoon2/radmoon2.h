#ifndef __RADMOON2_H_
#define __RADMOON2_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radmoon2/radmoon2base.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"

namespace icsneo {

class RADMoon2 : public RADMoon2Base {
public:
	// Serial numbers start with RM
	// USB PID is 0x1202, standard driver is DXX
	ICSNEO_FINDABLE_DEVICE(RADMoon2, DeviceType::RADMoon2, "RM");

	uint8_t getPhyAddrOrPort() const override { return 6; };


	ProductID getProductID() const override {
		return ProductID::RADMoon2;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADMoon2_ZYNQ, true, "ZCHIP", "RADMoon2_SW_bin", 0, FirmwareType::Zip},
			{ChipID::RADMoon2_Z7010_ZYNQ, false, "ZCHIP", "RADMoon2_Z7010_SW_bin", 0, FirmwareType::Zip}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADMoon2_ZYNQ, BootloaderCommunication::RAD)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}
protected:
	RADMoon2(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADMoon2Base(neodevice) {
		initialize<RADMoon2Settings>(makeDriver);
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}
};

}

#endif // __cplusplus

#endif