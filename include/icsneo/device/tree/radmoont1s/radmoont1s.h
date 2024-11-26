#ifndef __RADMOONT1S_H_
#define __RADMOONT1S_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/tree/radmoont1s/radmoont1ssettings.h"

namespace icsneo {

class RADMoonT1S : public Device {
public:

	// Serial numbers start with MS
	// USB PID is 0x1209, standard driver is FTDI3
	// Ethernet MAC allocation is 0x21, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADMoonT1S, _icsneo_devicetype_t::icsneo_devicetype_rad_moon_t1s, "MS");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::Ethernet,

			Network::NetID::OP_Ethernet1,

			Network::NetID::MDIO1,

			Network::NetID::SPI1,
		};
		return supportedNetworks;
	}


	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsTC10() const override { return true; }

protected:
	RADMoonT1S(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoonT1SSettings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
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
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override {
		setupSupportedRXNetworks(txNetworks);
	}
};

}

#endif // __cplusplus

#endif // __RADMOONT1S_H_
