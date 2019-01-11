#ifndef __VALUECAN4_1_H_
#define __VALUECAN4_1_H_

#include "icsneo/device/valuecan4/valuecan4.h"
#include "icsneo/device/valuecan4/settings/valuecan4-1settings.h"
#include <string>

namespace icsneo {

class ValueCAN4_1 : public ValueCAN4 {
public:
	// Serial numbers start with V1 for 4-1
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_1;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == "V1")
				found.emplace_back(new ValueCAN4_1(neodevice));
		}

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN
		};
		return supportedNetworks;
	}

protected:
	void setupEncoder(Encoder& encoder) override {
		ValueCAN4::setupEncoder(encoder);
		encoder.supportCANFD = false; // VCAN 4-1 does not support CAN FD
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

private:
	ValueCAN4_1(neodevice_t neodevice) : ValueCAN4(neodevice) {
		initialize<STM32, ValueCAN4_1Settings>();
		getWritableNeoDevice().type = DEVICE_TYPE;
	}
};

}

#endif