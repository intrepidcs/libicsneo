#ifndef __PLASION_H_
#define __PLASION_H_

#include "device/include/device.h"
#include "communication/include/multichannelcommunication.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class Plasion : public Device {
public:
	Plasion(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::make_shared<FTDI>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		auto decoder = std::make_shared<MessageDecoder>();
		com = std::make_shared<MultiChannelCommunication>(transport, packetizer, decoder);
	}
};

};

#endif