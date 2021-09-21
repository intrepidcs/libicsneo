#ifndef __ENCODER_H_
#define __ENCODER_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/packet.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/packetizer.h"
#include <queue>
#include <vector>
#include <memory>

namespace icsneo {

class Encoder {
public:
	Encoder(device_eventhandler_t report) : report(report) {}
	bool encode(const Packetizer& packetizer, std::vector<uint8_t>& result, const std::shared_ptr<Message>& message);
	bool encode(const Packetizer& packetizer, std::vector<uint8_t>& result, Command cmd, std::vector<uint8_t> arguments = {});

	bool supportCANFD = false;
private:
	device_eventhandler_t report;
};

}

#endif // __cplusplus

#endif