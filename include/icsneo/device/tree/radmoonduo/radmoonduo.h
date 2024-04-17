#ifndef __RADMOONDUO_H_
#define __RADMOONDUO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoonduo/radmoonduosettings.h"

namespace icsneo {

class RADMoonDuo : public Device {
public:
	// Serial numbers start with MD
	// USB PID is 1106, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADMoonDuo, DeviceType::RADMoonDuo, "MD");

	static const std::vector<Network>& GetSupportedNetworks() {
		// If Converter1 Target is set to USB/CM, OP_Ethernet2 will be exposed to the PC
		static std::vector<Network> supportedNetworks = {
			Network::NetID::OP_Ethernet2
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsComponentVersions() const override { return true; }

protected:
	RADMoonDuo(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoonDuoSettings>(makeDriver);
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