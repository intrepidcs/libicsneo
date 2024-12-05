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
	ICSNEO_FINDABLE_DEVICE(RADPluto, _icsneo_devicetype_t::icsneo_devicetype_rad_pluto, "PL");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,

			Network::_icsneo_netid_t::icsneo_netid_lin,

			Network::_icsneo_netid_t::icsneo_netid_ethernet,
			
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet1,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet2,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet3,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet4
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsComponentVersions() const override { return true; }

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