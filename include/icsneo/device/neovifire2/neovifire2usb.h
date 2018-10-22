#ifndef __NEOVIFIRE2USB_H_
#define __NEOVIFIRE2USB_H_

#include "icsneo/device/neovifire2/neovifire2.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/neovifire2/neovifire2settings.h"

namespace icsneo {

class NeoVIFIRE2USB : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1000;
	NeoVIFIRE2USB(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		com = MakeCommunication(std::unique_ptr<ICommunication>(new FTDI(getWritableNeoDevice())));
		settings = std::unique_ptr<IDeviceSettings>(new NeoVIFIRE2Settings(com));
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoVIFIRE2USB>(neodevice));

		return found;
	}
};

}

#endif