#ifndef __NEOOBD2PRO_H_
#define __NEOOBD2PRO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/stm32.h"

namespace icsneo {

class NeoOBD2PRO : public Device {
public:
	// Serial numbers are NP****
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::OBD2_PRO;
	static constexpr const uint16_t PRODUCT_ID = 0x1103;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoOBD2PRO(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2
		};
		return supportedNetworks;
	}

private:
	NeoOBD2PRO(neodevice_t neodevice) : Device(neodevice) {
		initialize<STM32>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }
};

}

#endif // __cplusplus

#endif