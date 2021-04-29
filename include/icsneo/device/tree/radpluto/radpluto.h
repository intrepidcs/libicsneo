#ifndef __RADPLUTO_H_
#define __RADPLUTO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/device/tree/radpluto/radplutosettings.h"

namespace icsneo {

class RADPluto : public Device {
public:
	// Serial numbers start with PL
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADPluto;
	static constexpr const uint16_t PRODUCT_ID = 0x1104;
	static constexpr const char* SERIAL_START = "PL";

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::LIN,

			Network::NetID::Ethernet,
			
			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,
			Network::NetID::OP_Ethernet3,
			Network::NetID::OP_Ethernet4
		};
		return supportedNetworks;
	}

	RADPluto(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

protected:
	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool requiresVehiclePower() const override { return false; }
};

}

#endif // __cplusplus

#endif