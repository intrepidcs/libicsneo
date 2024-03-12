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
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,
			Network::NetID::HSCAN3,
			Network::NetID::HSCAN4
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