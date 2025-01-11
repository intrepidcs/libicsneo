#ifndef __ETHERNETSTATUSMESSAGE_H__
#define __ETHERNETSTATUSMESSAGE_H__

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <memory>

namespace icsneo {

class EthernetStatusMessage : public Message {
public:
	enum class LinkSpeed {
		LinkSpeedAuto,
		LinkSpeed10,
		LinkSpeed100,
		LinkSpeed1000,
		LinkSpeed2500,
		LinkSpeed5000,
		LinkSpeed10000,
	};
	enum class LinkMode {
		LinkModeAuto,
		LinkModeMaster,
		LinkModeSlave,
		LinkModeInvalid,
	};
	EthernetStatusMessage(Network net, bool state, LinkSpeed speed, bool duplex, LinkMode mode) : Message(Type::EthernetStatus),
		network(net), state(state), speed(speed), duplex(duplex), mode(mode) {}
	Network network;
	bool state;
	LinkSpeed speed;
	bool duplex;
	LinkMode mode;
	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
};

}; // namespace icsneo

#endif // __cplusplus

#endif // __ETHERNETSTATUSMESSAGE_H__
