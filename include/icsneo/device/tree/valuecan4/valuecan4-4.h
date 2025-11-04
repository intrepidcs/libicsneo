#ifndef __VALUECAN4_4_H_
#define __VALUECAN4_4_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-4settings.h"

namespace icsneo {

class ValueCAN4_4 : public ValueCAN4 {
public:
	// Serial numbers start with V4 for 4-4
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(ValueCAN4_4, DeviceType::VCAN4_4, "V4");

	enum class SKU {
		Standard,
		AP0400A_D26, // HDB26, USB A, and Keysight Branding
		AP0400A_DB9, // 4xDB9, USB A, and Keysight Branding
		AP0400A_OBD, // OBD, USB A, and Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::AP0400A_D26;
			case 'B':
				return SKU::AP0400A_DB9;
			case 'C':
				return SKU::AP0400A_OBD;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP0400A_D26:
				return "Keysight AP0400A-D26";
			case SKU::AP0400A_DB9:
				return "Keysight AP0400A-DB9";
			case SKU::AP0400A_OBD:
				return "Keysight AP0400A-OBD";
		}
		return Device::getProductName();
	}

	ProductID getProductID() const override {
		return ProductID::ValueCAN4_2EL_4;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::ValueCAN4_4_MCHIP, true, "MCHIP", "vcan44_mchip_ief", 0, FirmwareType::IEF},
			{ChipID::ValueCAN4_4_SCHIP, true, "SCHIP", "vcan44_schip_ief", 1, FirmwareType::IEF},
			{ChipID::ValueCAN4_4_2EL_Core, true, "Core", "vcan44_core_ief", 2, FirmwareType::IEF}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::ValueCAN4_4_MCHIP, BootloaderCommunication::RED)
			.add<FlashPhase>(ChipID::ValueCAN4_4_SCHIP, BootloaderCommunication::RED)
			.add<FlashPhase>(ChipID::ValueCAN4_4_2EL_Core, BootloaderCommunication::REDCore)
			.add<EnterApplicationPhase>(ChipID::ValueCAN4_4_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}
protected:
	ValueCAN4_4(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4_4Settings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif // __cplusplus

#endif