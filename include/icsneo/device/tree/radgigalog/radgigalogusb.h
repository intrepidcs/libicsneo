#ifndef __RADGIGALOG_USB_H_
#define __RADGIGALOG_USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radgigalog/radgigalog.h"
#include "icsneo/platform/ftdi3.h"

namespace icsneo {

class RADGigalogUSB : public RADGigalog {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1203;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI3::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADGigalogUSB(neodevice)); // Creation of the shared_ptr

		return found;
	}

private:
	RADGigalogUSB(neodevice_t neodevice) : RADGigalog(neodevice) {
		initialize<FTDI3, RADGigalogSettings>();
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif