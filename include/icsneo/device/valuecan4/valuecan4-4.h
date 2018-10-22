#ifndef __VALUECAN4_4_H_
#define __VALUECAN4_4_H_

#include "icsneo/device/valuecan4/valuecan4.h"
#include "icsneo/device/valuecan4/settings/valuecan4-4settings.h"
#include <string>

namespace icsneo {

class ValueCAN4_4 : public ValueCAN4 {
public:
	// Serial numbers start with V4 for 4-4
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_4;
	ValueCAN4_4(neodevice_t neodevice) : ValueCAN4(neodevice) {
		com = MakeCommunication(getWritableNeoDevice());
		settings = std::unique_ptr<IDeviceSettings>(new ValueCAN4_4Settings(com));
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == "V4")
				found.push_back(std::make_shared<ValueCAN4_4>(neodevice));
		}

		return found;
	}
};

}

#endif