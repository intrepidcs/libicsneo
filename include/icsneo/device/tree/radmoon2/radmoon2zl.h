#ifndef __RADMOON2ZL_H_
#define __RADMOON2ZL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radmoon2/radmoon2base.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"

namespace icsneo {

class RADMoon2ZL : public RADMoon2Base {
public:
	// Serial numbers start with RN
	// USB PID is 0x110C, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADMoon2ZL, DeviceType::RADMoon2, "RN");

	uint8_t getPhyAddrOrPort() const override { return 1; }

	bool supportsTC10() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADMoon2;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADMoon2_ZL_MCHIP, true, "MCHIP", "radmoon2_zl_mchip_ief", 0, FirmwareType::IEF}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADMoon2_ZL_MCHIP, BootloaderCommunication::RED)
			.add<EnterApplicationPhase>(ChipID::RADMoon2_ZL_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}
protected:
	RADMoon2ZL(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADMoon2Base(neodevice) {
		initialize<RADMoon2Settings>(makeDriver);
	}
};

}

#endif // __cplusplus

#endif