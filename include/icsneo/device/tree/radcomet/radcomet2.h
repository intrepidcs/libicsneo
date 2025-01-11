#ifndef __RADCOMET2_H_
#define __RADCOMET2_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radcomet/radcometbase.h"
#include "icsneo/device/tree/radcomet/radcometsettings.h"

namespace icsneo {

class RADComet2 : public RADCometBase {
public:

	// Serial numbers start with RC, Comet2 starts at RC0300
	// USB PID is 0x1207, standard driver is FTDI3
	// Ethernet MAC allocation is 0x1D, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE_BY_SERIAL_RANGE(RADComet2, DeviceType::RADComet, "RC0300", "RCZZZZ");

	std::string getProductName() const override {
		return "RAD-Comet 2";
	}

	const std::vector<Network>& GetSupportedNetworks() override {
		static std::vector<Network> supportedNetworks = RADCometBase::GetSupportedNetworks();
		supportedNetworks.push_back(Network::NetID::OP_Ethernet3);
		supportedNetworks.push_back(Network::NetID::MDIO4);
		return supportedNetworks;
	}

	bool supportsTC10() const override { return true; }

protected:
	RADComet2(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADCometBase(neodevice) {
		initialize<RADCometSettings>(makeDriver);
	}

};

}

#endif // __cplusplus

#endif
