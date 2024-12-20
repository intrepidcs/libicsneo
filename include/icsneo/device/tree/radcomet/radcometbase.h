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
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet,

			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,

			Network::NetID::LIN,
			Network::NetID::ISO9141,

			Network::NetID::MDIO1,
			Network::NetID::MDIO2,
			Network::NetID::MDIO3,
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }
	bool supportsGPTP() const override { return true; }

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