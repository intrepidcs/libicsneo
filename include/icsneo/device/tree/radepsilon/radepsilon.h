#ifndef __RADEPSILON_H_
#define __RADEPSILON_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class RADEpsilon : public Device {
public:
	// Serial numbers start with RE
	// USB PID is 0x1109, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADEpsilon, _icsneo_devicetype_t::icsneo_devicetype_rad_epsilon, "RE");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,

			Network::_icsneo_netid_t::icsneo_netid_ethernet, // Connected to port 6 on the switch

			Network::_icsneo_netid_t::icsneo_netid_lin
		};
		return supportedNetworks;
	}

	bool supportsComponentVersions() const override { return true; }

protected:
	RADEpsilon(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize(makeDriver);
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 14*1024*1024;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif
