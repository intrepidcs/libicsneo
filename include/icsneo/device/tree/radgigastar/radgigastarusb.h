#ifndef __RADGIGASTAR_USB_H_
#define __RADGIGASTAR_USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radgigastar/radgigastar.h"
#include "icsneo/platform/ftdi3.h"

namespace icsneo {

class RADGigastarUSB : public RADGigastar {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1204;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI3::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADGigastarUSB(neodevice)); // Creation of the shared_ptr

		return found;
	}

private:
	RADGigastarUSB(neodevice_t neodevice) : RADGigastar(neodevice) {
		initialize<FTDI3, RADGigastarSettings>();
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif