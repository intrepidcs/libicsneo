#ifndef __VIVIDCAN_H_
#define __VIVIDCAN_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/cdcacm.h"
#include "icsneo/device/tree/vividcan/vividcansettings.h"

namespace icsneo {

class VividCAN : public Device {
public:
	// Serial numbers start with VV
	// USB PID is 0x1102, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(VividCAN, DeviceType::VividCAN, "VV");

	// VividCAN does not go online, you can only set settings
	bool goOnline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool goOffline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool isOnlineSupported() const override { return false; }

	ProductID getProductID() const override {
		return ProductID::VividCAN;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::VividCAN_MCHIP, false, "MCHIP", "vividcan_mchip_ief", 0, FirmwareType::IEF},
			{ChipID::VividCAN_EXT_FLASH, false, "Ext. Flash", "vividcan_ext_flash_ief", 0, FirmwareType::IEF}
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override { 
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::VividCAN_EXT_FLASH, BootloaderCommunication::RED, false)
			.add<FlashPhase>(ChipID::VividCAN_MCHIP, BootloaderCommunication::RED, false)
			.add<EnterApplicationPhase>(ChipID::VividCAN_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}
protected:
	VividCAN(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<VividCANSettings>(makeDriver);
	}

	bool requiresVehiclePower() const override { return false; }
	
	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*128;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif // __cplusplus

#endif