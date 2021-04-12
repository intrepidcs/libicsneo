#ifndef __VALUECAN4_4_H_
#define __VALUECAN4_4_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-4settings.h"
#include "icsneo/platform/stm32.h"
#include <string>

namespace icsneo {

class ValueCAN4_4 : public ValueCAN4 {
public:
	// Serial numbers start with V4 for 4-4
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_4;
	static constexpr const char* SERIAL_START = "V4";

	enum class SKU {
		Standard,
		AP0400A_D26, // HDB26, USB A, and Keysight Branding
		AP0400A_DB9, // 4xDB9, USB A, and Keysight Branding
		AP0400A_OBD, // OBD, USB A, and Keysight Branding
	};

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(USB_PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == SERIAL_START)
				found.emplace_back(new ValueCAN4_4(neodevice));
		}

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,
			Network::NetID::HSCAN3,
			Network::NetID::HSCAN4
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::AP0400A_D26;
			case 'B':
				return SKU::AP0400A_DB9;
			case 'C':
				return SKU::AP0400A_OBD;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP0400A_D26:
				return "Keysight AP0400A-D26";
			case SKU::AP0400A_DB9:
				return "Keysight AP0400A-DB9";
			case SKU::AP0400A_OBD:
				return "Keysight AP0400A-OBD";
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
	ValueCAN4_4(neodevice_t neodevice) : ValueCAN4(neodevice) {
		initialize<STM32, ValueCAN4_4Settings>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = USB_PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif