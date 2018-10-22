#ifndef __NEOVIION_H_
#define __NEOVIION_H_

#include "icsneo/device/plasion/plasion.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class NeoVIION : public Plasion {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::ION;
	static constexpr const uint16_t PRODUCT_ID = 0x0901;
	NeoVIION(neodevice_t neodevice) : Plasion(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
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