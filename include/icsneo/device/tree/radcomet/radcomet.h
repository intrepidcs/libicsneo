#ifndef __RADCOMET_H_
#define __RADCOMET_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/device/tree/radcomet/radcometsettings.h"

namespace icsneo {

class RADCOMET : public Device {
public:
	// Serial numbers start with RC
	// USB PID is 0x1207, standard driver is FTDI3
	// Ethernet MAC allocation is 0x1D, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADCOMET, DeviceType::RADComet, "RC");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet,

			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,

		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

protected:
	RADCOMET(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADCOMETSettings>(makeDriver);
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
};

}

#endif // __cplusplus

#endif