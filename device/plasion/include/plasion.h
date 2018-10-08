#ifndef __PLASION_H_
#define __PLASION_H_

#include "device/include/device.h"
#include "communication/include/multichannelcommunication.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class Plasion : public Device {
public:
	Plasion(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new FTDI(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<MultiChannelCommunication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
	}
};

}

#endif