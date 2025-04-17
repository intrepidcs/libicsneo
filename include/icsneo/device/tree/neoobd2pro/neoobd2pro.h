#ifndef __NEOOBD2PRO_H_
#define __NEOOBD2PRO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class NeoOBD2PRO : public Device {
public:
	// Serial numbers start with NP
	// USB PID is 0x1103, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(NeoOBD2PRO, DeviceType::OBD2_PRO, "NP");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02
		};
		return supportedNetworks;
	}

private:
	NeoOBD2PRO(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool requiresVehiclePower() const override { return false; }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 2048 * 512;
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