#ifndef __NEOVIFIRE_H_
#define __NEOVIFIRE_H_

#include "device/include/device.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class NeoVIFIRE : public Device {
public:
	static constexpr const char* PRODUCT_NAME = "neoVI FIRE";
	static constexpr const uint16_t PRODUCT_ID = 0x0701;
	NeoVIFIRE(neodevice_t neodevice) : Device(neodevice) {
		com = std::make_shared<Communication>(std::make_shared<FTDI>(getWritableNeoDevice()));
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	enum class Mode : char {
		Application = 'A',
		Bootloader = 'B'
	};

	bool goOnline() {
		// Enter mode is only needed on very old FIRE devices, will be ignored by newer devices
		if(!enterMode(Mode::Application))
			return false;

		return Device::goOnline();
	}

	bool enterMode(Mode mode) {
		// Included for compatibility with bootloaders on very old FIRE devices
		// Mode will be a uppercase char like 'A'
		if(!com->rawWrite({ (uint8_t)mode }))
			return false;

		// We then expect to see that same mode back in lowercase
		// This won't happen in the case of new devices, though, so we assume it worked
		return true;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoVIFIRE>(neodevice));

		return found;
	}
};

};

#endif