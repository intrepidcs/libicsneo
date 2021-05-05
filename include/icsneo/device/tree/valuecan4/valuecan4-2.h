#ifndef __VALUECAN4_2_H_
#define __VALUECAN4_2_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-2settings.h"
#include "icsneo/platform/cdcacm.h"
#include <string>

namespace icsneo {

class ValueCAN4_2 : public ValueCAN4 {
public:
	// Serial numbers start with V2 for 4-2
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_2;
	static constexpr const char* SERIAL_START = "V2";

	enum class SKU {
		Standard,
		AP0200A, // USB A and Keysight Branding
	};

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(USB_PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == SERIAL_START)
				found.emplace_back(new ValueCAN4_2(neodevice));
		}

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
			case 'B':
				return SKU::AP0200A;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP0200A:
				return "Keysight AP0200A";
		}
		return Device::getProductName();
	}

protected:
	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

private:
	ValueCAN4_2(neodevice_t neodevice) : ValueCAN4(neodevice) {
		initialize<CDCACM, ValueCAN4_2Settings>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = USB_PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif