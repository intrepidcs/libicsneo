#ifndef __VALUECAN4_1_H_
#define __VALUECAN4_1_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-1settings.h"

namespace icsneo {

class ValueCAN4_1 : public ValueCAN4 {
public:
	// Serial numbers start with V1 for 4-1
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(ValueCAN4_1, DeviceType::VCAN4_1, "V1");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN
		};
		return supportedNetworks;
	}

protected:
	ValueCAN4_1(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4_1Settings>(makeDriver);
	}

	void setupEncoder(Encoder& encoder) override {
		ValueCAN4::setupEncoder(encoder);
		encoder.supportCANFD = false; // VCAN 4-1 does not support CAN FD
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }
};

}

#endif // __cplusplus

#endif