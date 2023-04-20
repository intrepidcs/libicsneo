#ifndef __RADMOON3_H_
#define __RADMOON3_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoon3/radmoon3settings.h"

namespace icsneo {

class RADMoon3 : public Device {
public:
	// Serial numbers start with R3
	// USB PID is 0x110D, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADMoon3, DeviceType::RADMoon3, "R3");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::Ethernet,
			Network::NetID::OP_Ethernet1,
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

protected:
	RADMoon3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoon3Settings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.align16bit = true;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportEthPhy = true;
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