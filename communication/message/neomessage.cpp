#include "icsneo/communication/message/neomessage.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"

using namespace icsneo;

neomessage_t icsneo::CreateNeoMessage(const std::shared_ptr<Message> message) {
	// This function is not responsible for storing the message!
	// Keep the shared_ptr around for the lifetime of the data access
	const auto type = message->network.getType();
	neomessage_t neomsg = {}; // Clear out the memory
	neomsg.netid = (uint32_t)message->network.getNetID();
	neomsg.type = (uint8_t)type;
	neomsg.length = message->data.size();
	neomsg.data = message->data.data();
	neomsg.timestamp = message->timestamp;
	neomsg.timestampMultiplier = message->timestampMultiplier;
	neomsg.status.globalError = message->error;
	neomsg.status.transmitMessage = message->transmitted;

	switch(type) {
		case Network::Type::CAN:
		case Network::Type::SWCAN:
		case Network::Type::LSFTCAN: {
			neomessage_can_t& can = *(neomessage_can_t*)&neomsg;
			auto canmsg = std::static_pointer_cast<CANMessage>(message);
			can.arbid = canmsg->arbid;
			can.dlcOnWire = canmsg->dlcOnWire;
			can.status.extendedFrame = canmsg->isExtended;
			can.status.remoteFrame = canmsg->isRemote;
			can.status.canfdRTR = canmsg->isRemote;
			can.status.canfdFDF = canmsg->isCANFD;
			can.status.canfdBRS = canmsg->baudrateSwitch;
			break;
		}
		case Network::Type::Ethernet: {
			neomessage_eth_t& eth = *(neomessage_eth_t*)&neomsg;
			auto ethmsg = std::static_pointer_cast<EthernetMessage>(message);
			eth.preemptionFlags = ethmsg->preemptionFlags;
			eth.status.incompleteFrame = ethmsg->frameTooShort;
			// TODO Fill in extra status bits
			//eth.status.xyz = ethmsg->preemptionEnabled;
			//eth.status.xyz = ethmsg->fcsAvailable;
			//eth.status.xyz = ethmsg->noPadding;
			break;
		}
		default:
			// TODO Implement others
			break;
	}

	return neomsg;
}

std::shared_ptr<Message> icsneo::CreateMessageFromNeoMessage(const neomessage_t* neomessage) {
	const Network network = neomessage->netid;
	switch(network.getType()) {
		case Network::Type::CAN:
		case Network::Type::SWCAN:
		case Network::Type::LSFTCAN: {
			neomessage_can_t& can = *(neomessage_can_t*)neomessage;
			auto canmsg = std::make_shared<CANMessage>();
			canmsg->network = network;
			canmsg->data.insert(canmsg->data.end(), can.data, can.data + can.length);
			canmsg->arbid = can.arbid;
			canmsg->isExtended = can.status.extendedFrame;
			canmsg->isRemote = can.status.remoteFrame | can.status.canfdRTR;
			canmsg->isCANFD = can.status.canfdFDF;
			canmsg->baudrateSwitch = can.status.canfdBRS;
			return canmsg;
		}
		default:
			// TODO Implement others
			return std::shared_ptr<Message>();
	}
}