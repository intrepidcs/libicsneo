#include "icsneo/communication/message/ethernetstatusmessage.h"

using namespace icsneo;

#pragma pack(push, 1)
enum LinkSpeed {
	ethSpeed10,
	ethSpeed100,
	ethSpeed1000,
	ethSpeedAutoNeg,
	ethSpeed2500,
	ethSpeed5000,
	ethSpeed10000,
};

enum TLinkMode {
	T_LINK_NONE,
	T_LINK_MASTER,
	T_LINK_SLAVE,
	T_LINK_AUTO,
};

enum AELinkMode {
	AE_LINK_AUTO,
	AE_LINK_MASTER,
	AE_LINK_SLAVE,
	AE_LINK_INVALID = 255,
};

struct Packet {
	uint8_t state;
	uint8_t speed;
	uint8_t duplex;
	uint16_t network;
	uint8_t mode;
};
#pragma pack(pop)

std::shared_ptr<Message> EthernetStatusMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(Packet)) {
		return nullptr;
	}
	Packet* packet = (Packet*)bytestream.data();
	LinkSpeed speed;
	switch(packet->speed) {
		case ethSpeed10: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed10; break;
		case ethSpeed100: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed100; break;
		case ethSpeed1000: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed1000; break;
		case ethSpeedAutoNeg: speed = EthernetStatusMessage::LinkSpeed::LinkSpeedAuto; break;
		case ethSpeed2500: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed2500; break;
		case ethSpeed5000: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed5000; break;
		case ethSpeed10000: speed = EthernetStatusMessage::LinkSpeed::LinkSpeed10000; break;
		default: return nullptr;
	}
	LinkMode mode;
	switch(Network::GetTypeOfNetID((Network::NetID)packet->network, false)) {
		case Network::Type::Ethernet:
			switch(packet->mode) {
				case T_LINK_NONE: mode = EthernetStatusMessage::LinkMode::LinkModeNone; break;
				case T_LINK_MASTER: mode = EthernetStatusMessage::LinkMode::LinkModeMaster; break;
				case T_LINK_SLAVE: mode = EthernetStatusMessage::LinkMode::LinkModeSlave; break;
				case T_LINK_AUTO: mode = EthernetStatusMessage::LinkMode::LinkModeAuto; break;
				default: return nullptr;
			}
			break;
		case Network::Type::AutomotiveEthernet:
			switch(packet->mode) {
				case AE_LINK_AUTO: mode = EthernetStatusMessage::LinkMode::LinkModeAuto; break;
				case AE_LINK_MASTER: mode = EthernetStatusMessage::LinkMode::LinkModeMaster; break;
				case AE_LINK_SLAVE: mode = EthernetStatusMessage::LinkMode::LinkModeSlave; break;
				case AE_LINK_INVALID: mode = EthernetStatusMessage::LinkMode::LinkModeInvalid; break;
				default: return nullptr;
			}
			break;
		default: return nullptr;
	}
	return std::make_shared<EthernetStatusMessage>(packet->network, packet->state, speed, packet->duplex, mode);
}
