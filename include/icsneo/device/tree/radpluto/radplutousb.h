#ifndef __RADPLUTOUSB_H_
#define __RADPLUTOUSB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radpluto/radpluto.h"
#include "icsneo/platform/cdcacm.h"

namespace icsneo {

class RADPlutoUSB : public RADPluto {
public:
	// Serial numbers start with RP
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADPlutoUSB(neodevice));

		return found;
	}

private:
	RADPlutoUSB(neodevice_t neodevice) : RADPluto(neodevice) {
		initialize<CDCACM, RADPlutoSettings>();
	}
};

}

#endif // __cplusplus

#endif