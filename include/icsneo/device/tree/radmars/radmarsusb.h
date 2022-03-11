#ifndef __RADMARS_USB_H_
#define __RADMARS_USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radmars/radmars.h"
#include "icsneo/platform/ftdi3.h"

namespace icsneo {

class RADMarsUSB : public RADMars {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1203;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI3::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADMarsUSB(neodevice)); // Creation of the shared_ptr

		return found;
	}

private:
	RADMarsUSB(neodevice_t neodevice) : RADMars(neodevice) {
		initialize<FTDI3, RADMarsSettings>();
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif