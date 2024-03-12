#ifndef __RADPLUTO_H_
#define __RADPLUTO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radpluto/radplutosettings.h"

namespace icsneo {

class RADPluto : public Device {
public:
	// Serial numbers start with PL
	// USB PID is 1104, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADPluto, DeviceType::RADPluto, "PL");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::LIN,

			Network::NetID::Ethernet,
			
			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,
			Network::NetID::OP_Ethernet3,
			Network::NetID::OP_Ethernet4
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

protected:
	RADPluto(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADPlutoSettings>(makeDriver);
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
		encoder.supportEthPhy = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool requiresVehiclePower() const override { return false; }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*2048;
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