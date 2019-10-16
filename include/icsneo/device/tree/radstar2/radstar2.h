#ifndef __RADSTAR2_H_
#define __RADSTAR2_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radstar2/radstar2settings.h"

namespace icsneo {

class RADStar2 : public Device {
public:
	// Serial numbers start with RS
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADStar2;
	static constexpr const uint16_t PRODUCT_ID = 0x0005;
	static constexpr const char* SERIAL_START = "RS";

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::MSCAN,

			Network::NetID::LIN,

			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2
		};
		return supportedNetworks;
	}

protected:
	virtual void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	virtual void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	RADStar2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif