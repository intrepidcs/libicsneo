#ifndef __RADCOMETBASE_H_
#define __RADCOMETBASE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class RADCometBase : public Device {
public:
	virtual const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::HSCAN2,

			Network::_icsneo_netid_t::Ethernet,

			Network::_icsneo_netid_t::icsneo_netid_op_ethernet1,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet2,

			Network::_icsneo_netid_t::icsneo_netid_lin,
			Network::_icsneo_netid_t::icsneo_netid_iso9141,

			Network::_icsneo_netid_t::MDIO1,
			Network::_icsneo_netid_t::MDIO2,
			Network::_icsneo_netid_t::MDIO3,
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

protected:
	using Device::Device;

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