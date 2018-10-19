#ifndef __VALUECAN4_1_H_
#define __VALUECAN4_1_H_

#include "device/valuecan4/include/valuecan4.h"
#include "device/valuecan4/settings/include/valuecan4-1settings.h"
#include <string>

namespace icsneo {

class ValueCAN4_1 : public ValueCAN4 {
public:
	// Serial numbers start with V1 for 4-1
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_1;
	ValueCAN4_1(neodevice_t neodevice) : ValueCAN4(neodevice) {
		com = MakeCommunication(getWritableNeoDevice());
		com->encoder->supportCANFD = false; // VCAN 4-1 does not support CAN FD
		settings = std::unique_ptr<IDeviceSettings>(new ValueCAN4_1Settings(com));
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == "V1")
				found.push_back(std::make_shared<ValueCAN4_1>(neodevice));
		}

		return found;
	}
};

}

#endif