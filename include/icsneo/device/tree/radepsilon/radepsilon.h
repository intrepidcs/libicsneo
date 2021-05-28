#ifndef __RADEPSILON_H_
#define __RADEPSILON_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/cdcacm.h"

namespace icsneo {

class RADEpsilon : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADEpsilon;
	static constexpr const char* SERIAL_START = "RE";
	static constexpr const uint16_t PRODUCT_ID = 0x1109;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADEpsilon(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet, // Connected to port 6 on the switch

			Network::NetID::LIN
		};
		return supportedNetworks;
	}

protected:
	RADEpsilon(neodevice_t neodevice) : Device(neodevice) {
		initialize<CDCACM>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

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
};

}

#endif
