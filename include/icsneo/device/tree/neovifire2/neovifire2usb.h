#ifndef __NEOVIFIRE2USB_H_
#define __NEOVIFIRE2USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/neovifire2/neovifire2.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/tree/neovifire2/neovifire2settings.h"

namespace icsneo {

class NeoVIFIRE2USB : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1000;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoVIFIRE2USB(neodevice)); // Creation of the shared_ptr

		return found;
	}

private:
	NeoVIFIRE2USB(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		initialize<FTDI, NeoVIFIRE2Settings>();
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif