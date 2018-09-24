#ifndef __PLASION_H_
#define __PLASION_H_

#include "device/include/device.h"
#include "communication/include/multichannelcommunication.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class Plasion : public Device {
public:
	Plasion(neodevice_t neodevice) : Device(neodevice) {
		com = std::make_shared<MultiChannelCommunication>(std::make_shared<FTDI>(getWritableNeoDevice()));
	}
};

}

#endif