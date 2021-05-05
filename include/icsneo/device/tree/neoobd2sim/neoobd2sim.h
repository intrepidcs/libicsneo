#ifndef __NEOOBD2SIM_H_
#define __NEOOBD2SIM_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/cdcacm.h"

namespace icsneo {

class NeoOBD2SIM : public Device {
public:
	// Serial numbers are OS****
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::OBD2_SIM;
	static constexpr const uint16_t PRODUCT_ID = 0x1100;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoOBD2SIM(neodevice));

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
	NeoOBD2SIM(neodevice_t neodevice) : Device(neodevice) {
		initialize<CDCACM>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
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