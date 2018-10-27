#ifndef __PLASION_H_
#define __PLASION_H_

#include "icsneo/device/device.h"
#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class Plasion : public Device {
protected:
	virtual std::shared_ptr<Communication> makeCommunication(
		std::unique_ptr<ICommunication> transport,
		std::shared_ptr<Packetizer> packetizer,
		std::unique_ptr<Encoder> encoder,
		std::unique_ptr<Decoder> decoder
	) override { return std::make_shared<MultiChannelCommunication>(err, std::move(transport), packetizer, std::move(encoder), std::move(decoder)); }

public:
	Plasion(neodevice_t neodevice) : Device(neodevice) {}
};

}

#endif