#ifndef __VIVIDCAN_H_
#define __VIVIDCAN_H_

#include "device/include/device.h"
#include "platform/include/stm32.h"

namespace icsneo {

class VividCAN : public Device {
public:
	// Serial numbers start with VV
	static constexpr const char* PRODUCT_NAME = "VividCAN";
	static constexpr const uint16_t USB_PRODUCT_ID = 0x1102;
	VividCAN(neodevice_t neodevice) : Device(neodevice) {
		com = std::make_shared<Communication>(std::make_shared<STM32>(getWritableNeoDevice()));
		setProductName(PRODUCT_NAME);
		usbProductId = USB_PRODUCT_ID;
	}

	bool goOnline() { return false; }

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(USB_PRODUCT_ID))
			found.push_back(std::make_shared<VividCAN>(neodevice));

		return found;
	}
};

};

#endif