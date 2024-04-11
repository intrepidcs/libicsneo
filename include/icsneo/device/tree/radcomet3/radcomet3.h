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
	ICSNEO_FINDABLE_DEVICE(RADComet3, DeviceType::RADComet3, "C3");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet,

			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,
			Network::NetID::OP_Ethernet3,
			Network::NetID::OP_Ethernet4,
			Network::NetID::OP_Ethernet5,
			Network::NetID::OP_Ethernet6,
			Network::NetID::OP_Ethernet7,

			Network::NetID::LIN,
			Network::NetID::ISO9141,

			Network::NetID::MDIO1,
			Network::NetID::MDIO2,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }

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
