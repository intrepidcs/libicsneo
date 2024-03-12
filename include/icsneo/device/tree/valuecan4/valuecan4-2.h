#ifndef __VALUECAN4_2_H_
#define __VALUECAN4_2_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-2settings.h"

namespace icsneo {

class ValueCAN4_2 : public ValueCAN4 {
public:
	// Serial numbers start with V2 for 4-2
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(ValueCAN4_2, DeviceType::VCAN4_2, "V2");

	enum class SKU {
		Standard,
		AP0200A, // USB A and Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
			case 'B':
				return SKU::AP0200A;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP0200A:
				return "Keysight AP0200A";
		}
		return Device::getProductName();
	}

protected:
	ValueCAN4_2(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4_2Settings>(makeDriver);
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