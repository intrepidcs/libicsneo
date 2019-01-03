#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::FIRE2;
	static constexpr const char* SERIAL_START = "CY";

	static constexpr Network::NetID SUPPORTED_NETWORKS[] = {
		Network::NetID::HSCAN,
		Network::NetID::MSCAN,
		Network::NetID::HSCAN2,
		Network::NetID::HSCAN3,
		Network::NetID::HSCAN4,
		Network::NetID::HSCAN5,
		Network::NetID::HSCAN6,
		Network::NetID::HSCAN7,

		Network::NetID::LSFTCAN,
		Network::NetID::LSFTCAN2,

		Network::NetID::SWCAN,
		Network::NetID::SWCAN2,

		Network::NetID::Ethernet,

		Network::NetID::LIN,
		Network::NetID::LIN2,
		Network::NetID::LIN3,
		Network::NetID::LIN4
	};

protected:
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : SUPPORTED_NETWORKS)
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }
};

}

#endif