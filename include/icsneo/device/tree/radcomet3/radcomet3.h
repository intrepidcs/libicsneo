#ifndef __RADCOMET3_H_
#define __RADCOMET3_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radcomet3/radcomet3.h"
#include "icsneo/device/tree/radcomet3/radcomet3settings.h"

namespace icsneo {

class RADComet3 : public Device {
public:

	// Serial numbers start with C3
	// USB PID is 0x1208, standard driver is FTDI3
	// Ethernet MAC allocation is 0x20, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADComet3, _icsneo_devicetype_t::icsneo_devicetype_rad_comet3, "C3");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,

			Network::_icsneo_netid_t::icsneo_netid_ethernet,

			Network::_icsneo_netid_t::icsneo_netid_op_ethernet1,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet2,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet3,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet4,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet5,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet6,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet7,

			Network::_icsneo_netid_t::icsneo_netid_lin,
			Network::_icsneo_netid_t::icsneo_netid_iso9141,

			Network::_icsneo_netid_t::MDIO1,
			Network::_icsneo_netid_t::MDIO2,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsTC10() const override { return true; }

protected:
	RADComet3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADComet3Settings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;		
		encoder.supportEthPhy = true;
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 32*1024*1024;
	}

};

}

#endif // __cplusplus

#endif
