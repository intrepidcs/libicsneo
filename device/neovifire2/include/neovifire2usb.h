#ifndef __NEOVIFIRE2USB_H_
#define __NEOVIFIRE2USB_H_

#include "device/neovifire2/include/neovifire2.h"
#include "platform/include/ftdi.h"
#include "device/neovifire2/include/neovifire2settings.h"

namespace icsneo {

class NeoVIFIRE2USB : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1000;
	NeoVIFIRE2USB(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		auto transport = std::make_shared<FTDI>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		auto decoder = std::make_shared<Decoder>();
		com = std::make_shared<Communication>(transport, packetizer, decoder);
		settings = std::make_shared<NeoVIFIRE2Settings>(com);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoVIFIRE2USB>(neodevice));

		return found;
	}
};

};

#endif