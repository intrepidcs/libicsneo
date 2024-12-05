#ifndef __VALUECAN4INDUSTRIAL_H_
#define __VALUECAN4INDUSTRIAL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4industrialsettings.h"

namespace icsneo {

class ValueCAN4Industrial : public ValueCAN4 {
public:
	// Serial numbers start with IV for Industrial
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	// Ethernet MAC allocation is 0x12, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(ValueCAN4Industrial, _icsneo_devicetype_t::icsneo_devicetype_vcan4_industrial, "IV");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,

			Network::_icsneo_netid_t::Ethernet,

			Network::_icsneo_netid_t::icsneo_netid_lin
		};
		return supportedNetworks;
	}

protected:
	ValueCAN4Industrial(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4IndustrialSettings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void setupPacketizer(Packetizer& packetizer) override {
		ValueCAN4::setupPacketizer(packetizer);
		packetizer.align16bit = !com->driver->isEthernet();
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif // __cplusplus

#endif