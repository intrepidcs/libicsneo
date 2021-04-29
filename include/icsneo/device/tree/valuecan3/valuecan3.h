#ifndef __VALUECAN3_H_
#define __VALUECAN3_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/tree/valuecan3/valuecan3settings.h"

namespace icsneo {

class ValueCAN3 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN3;
	static constexpr const uint16_t PRODUCT_ID = 0x0601;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new ValueCAN3(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::MSCAN
		};
		return supportedNetworks;
	}

private:
	ValueCAN3(neodevice_t neodevice) : Device(neodevice) {
		initialize<FTDI, ValueCAN3Settings>();
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