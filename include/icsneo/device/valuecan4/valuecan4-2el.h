#ifndef __VALUECAN4_2EL_H_
#define __VALUECAN4_2EL_H_

#include "icsneo/device/valuecan4/valuecan4.h"
#include "icsneo/device/valuecan4/settings/valuecan4-2elsettings.h"
#include <string>

namespace icsneo {

class ValueCAN4_2EL : public ValueCAN4 {
public:
	// Serial numbers start with VE for 4-2EL
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_2EL;
	ValueCAN4_2EL(neodevice_t neodevice) : ValueCAN4(neodevice) {
		com = MakeCommunication(getWritableNeoDevice());
		settings = std::unique_ptr<IDeviceSettings>(new ValueCAN4_2ELSettings(com));
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == "VE")
				found.push_back(std::make_shared<ValueCAN4_2EL>(neodevice));
		}

		return found;
	}
};

}

#endif