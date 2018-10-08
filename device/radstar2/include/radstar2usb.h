#ifndef __RADSTAR2USB_H_
#define __RADSTAR2USB_H_

#include "device/radstar2/include/radstar2.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class RADStar2USB : public RADStar2 {
public:
	// Serial numbers start with RS
	RADStar2USB(neodevice_t neodevice) : RADStar2(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new FTDI(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<RADStar2USB>(neodevice));

		return found;
	}
};

}

#endif