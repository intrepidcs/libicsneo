#ifndef __NEOVIION_H_
#define __NEOVIION_H_

#ifdef __cplusplus

#include "icsneo/device/tree/plasion/plasion.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class NeoVIION : public Plasion {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::ION;
	static constexpr const uint16_t PRODUCT_ID = 0x0901;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoVIION(neodevice));

		return found;
	}

private:
	NeoVIION(neodevice_t neodevice) : Plasion(neodevice) {
		initialize<FTDI>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif