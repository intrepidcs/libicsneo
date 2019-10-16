#ifndef __RADSTAR2USB_H_
#define __RADSTAR2USB_H_

#include "icsneo/device/tree/radstar2/radstar2.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class RADStar2USB : public RADStar2 {
public:
	// Serial numbers start with RS
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADStar2USB(neodevice));

		return found;
	}

private:
	RADStar2USB(neodevice_t neodevice) : RADStar2(neodevice) {
		initialize<FTDI, RADStar2Settings>();
	}
};

}

#endif