#ifndef __VALUECAN4_2EL_USB_H_
#define __VALUECAN4_2EL_USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4-2el.h"
#include "icsneo/platform/cdcacm.h"
#include <string>

namespace icsneo {

class ValueCAN4_2EL_USB : public ValueCAN4_2EL {
public:
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(USB_PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == SERIAL_START)
				found.emplace_back(new ValueCAN4_2EL_USB(neodevice));
		}

		return found;
	}

private:
	ValueCAN4_2EL_USB(neodevice_t neodevice) : ValueCAN4_2EL(neodevice) {
		initialize<CDCACM, ValueCAN4_2ELSettings>();
		productId = USB_PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif