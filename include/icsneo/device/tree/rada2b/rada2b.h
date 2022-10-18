#ifndef __RADA2B_H_
#define __RADA2B_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/device/tree/rada2b/rada2bsettings.h"

namespace icsneo {

class RADA2B : public Device {
public:
	// Serial numbers start with AB
	// USB PID is 0x0006, standard driver is FTDI
	// Ethernet MAC allocation is 0x18, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADA2B, DeviceType::RAD_A2B, "AB");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet,

			Network::NetID::LIN,

			Network::NetID::A2B1,
			Network::NetID::A2B2,

			Network::NetID::I2C,
			Network::NetID::I2C2
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

protected:
	RADA2B(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADA2BSettings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
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