#ifndef __ETHERNETSTATUSMESSAGE_H__
#define __ETHERNETSTATUSMESSAGE_H__

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/icsneoc2types.h"

#include <memory>

namespace icsneo {

class EthernetStatusMessage : public RawMessage {
public:
	enum class LinkSpeed: icsneoc2_link_speed_t {
		LinkSpeedAuto = icsneoc2_link_speed_auto,
		LinkSpeed10 = icsneoc2_link_speed_10mbps,
		LinkSpeed100 = icsneoc2_link_speed_100mbps,
		LinkSpeed1000 = icsneoc2_link_speed_1000mbps,
		LinkSpeed2500 = icsneoc2_link_speed_2500mbps,
		LinkSpeed5000 = icsneoc2_link_speed_5000mbps,
		LinkSpeed10000 = icsneoc2_link_speed_10000mbps,
	};
	enum class LinkMode: icsneoc2_link_mode_t {
		LinkModeAuto = icsneoc2_link_mode_auto,
		LinkModeMaster = icsneoc2_link_mode_master,
		LinkModeSlave = icsneoc2_link_mode_slave,
		LinkModeInvalid = icsneoc2_link_mode_invalid,
		LinkModeNone = icsneoc2_link_mode_none,
	};

	EthernetStatusMessage(Network net, bool state, LinkSpeed speed, bool duplex, LinkMode mode) : RawMessage(Type::EthernetStatus, net),
		state(state), speed(speed), duplex(duplex), mode(mode) {}
	// Link State: False = Link Down, True = Link Up
	bool state;
	LinkSpeed speed;
	// Duplex: False = Half, True = Full
	bool duplex;
	LinkMode mode;
	
	static std::shared_ptr<RawMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
};

}; // namespace icsneo

#endif // __cplusplus

#endif // __ETHERNETSTATUSMESSAGE_H__
