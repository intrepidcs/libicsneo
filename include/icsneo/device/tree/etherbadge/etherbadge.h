#ifndef __ETHERBADGE_H_
#define __ETHERBADGE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/platform/cdcacm.h"
#include "icsneo/device/tree/etherbadge/etherbadgesettings.h"

namespace icsneo {

class EtherBADGE : public Device {
public:
	// Serial numbers start with EB
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::EtherBADGE;
	static constexpr const uint16_t PRODUCT_ID = 0x1107;
	static constexpr const char* SERIAL_START = "EB";

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new EtherBADGE(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,

			Network::NetID::LIN,
			
			Network::NetID::OP_Ethernet1
		};
		return supportedNetworks;
	}

	EtherBADGE(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
		initialize<CDCACM, EtherBADGESettings>();
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