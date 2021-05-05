#ifndef __RADMOONDUO_H_
#define __RADMOONDUO_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoonduo/radmoonduosettings.h"
#include "icsneo/platform/cdcacm.h"

namespace icsneo {

class RADMoonDuo : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADMoonDuo;
	static constexpr const uint16_t PRODUCT_ID = 0x1106;
	static constexpr const char* SERIAL_START = "MD";

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADMoonDuo(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		// If Converter1 Target is set to USB/CM, OP_Ethernet2 will be exposed to the PC
		static std::vector<Network> supportedNetworks = {
			Network::NetID::OP_Ethernet2
		};
		return supportedNetworks;
	}

protected:
	RADMoonDuo(neodevice_t neodevice) : Device(neodevice) {
		initialize<CDCACM, RADMoonDuoSettings>();
		productId = PRODUCT_ID;
		getWritableNeoDevice().type = DEVICE_TYPE;
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