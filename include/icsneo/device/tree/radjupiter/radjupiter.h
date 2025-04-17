#ifndef __RADJUPITER_H_
#define __RADJUPITER_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radjupiter/radjupitersettings.h"

namespace icsneo {

class RADJupiter : public Device {
public:
	// Serial numbers start with RJ
	// USB PID is 1105, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADJupiter, DeviceType::RADJupiter, "RJ");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::LIN_01,

			Network::NetID::ETHERNET_01 // Connected to port 6 on the switch
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsComponentVersions() const override { return true; }

protected:
	RADJupiter(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADJupiterSettings>(makeDriver);
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