#ifndef __NEOVIION_H_
#define __NEOVIION_H_

#include "device/plasion/include/plasion.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class NeoVIION : public Plasion {
public:
	static constexpr const char* PRODUCT_NAME = "neoVI ION";
	static constexpr const uint16_t PRODUCT_ID = 0x0901;
	NeoVIION(neodevice_t neodevice) : Plasion(neodevice) {
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoVIION>(neodevice));

		return found;
	}
};

}

#endif