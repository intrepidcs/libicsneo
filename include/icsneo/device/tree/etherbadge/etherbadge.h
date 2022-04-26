#ifndef __ETHERBADGE_H_
#define __ETHERBADGE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/device/tree/etherbadge/etherbadgesettings.h"

namespace icsneo {

class EtherBADGE : public Device {
public:
	// Serial numbers start with EB
	// USB PID is 0x1107, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(EtherBADGE, DeviceType::EtherBADGE, "EB");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::LIN,
			Network::NetID::OP_Ethernet1
		};
		return supportedNetworks;
	}

protected:
	EtherBADGE(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<EtherBADGESettings>(makeDriver);
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool requiresVehiclePower() const override { return false; }
};

}

#endif // __cplusplus

#endif