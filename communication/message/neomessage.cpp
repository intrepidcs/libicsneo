#include "icsneo/communication/message/neomessage.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/message/canerrorcountmessage.h"

using namespace icsneo;

neomessage_t icsneo::CreateNeoMessage(const std::shared_ptr<Message> message) {
	// This function is not responsible for storing the message!
	// Keep the shared_ptr around for the lifetime of the data access
	neomessage_t neomsg = {}; // Clear out the memory
	neomsg.messageType = (neomessagetype_t)message->type;
	neomsg.timestamp = message->timestamp;
	switch (message->type)
	{
	case Message::Type::Frame: {
		neomessage_frame_t& frame = *(neomessage_frame_t*)&neomsg;
		auto framemsg = std::static_pointer_cast<Frame>(message);
		const auto netType = framemsg->network.getType();
		frame.netid = (neonetid_t)framemsg->network.getNetID();
		frame.type = (neonettype_t)netType;
		frame.description = framemsg->description;
		frame.length = framemsg->data.size();
		frame.data = framemsg->data.data();
		frame.timestamp = framemsg->timestamp;
		frame.status.globalError = framemsg->error;
		frame.status.transmitMessage = framemsg->transmitted;

		switch(netType) {
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
				can.status.canfdESI = canmsg->errorStateIndicator;
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
		break;
	}
	case Message::Type::CANErrorCount: {
		neomessage_can_error_t& canerror = *(neomessage_can_error_t*)&neomsg;
		auto canerrormsg = std::static_pointer_cast<CANErrorCountMessage>(message);
		canerror.transmitErrorCount = canerrormsg->transmitErrorCount;
		canerror.receiveErrorCount = canerrormsg->receiveErrorCount;
		canerror.status.canBusOff = canerrormsg->busOff;
		canerror.netid = (neonetid_t)canerrormsg->network.getNetID();
		canerror.type = (neonettype_t)canerrormsg->network.getType();
		break;
	}
	default:
		break;
	}
	return neomsg;
}

std::shared_ptr<Message> icsneo::CreateMessageFromNeoMessage(const neomessage_t* neomessage) {
	switch((Message::Type)neomessage->messageType) {
		case Message::Type::Frame: {
			const Network network = ((neomessage_frame_t*)neomessage)->netid;
			switch(network.getType()) {
				case Network::Type::CAN:
				case Network::Type::SWCAN:
				case Network::Type::LSFTCAN: {
					neomessage_can_t& can = *(neomessage_can_t*)neomessage;
					auto canmsg = std::make_shared<CANMessage>();
					canmsg->network = network;
					canmsg->description = can.description;
					canmsg->data.insert(canmsg->data.end(), can.data, can.data + can.length);
					canmsg->arbid = can.arbid;
					canmsg->dlcOnWire = can.dlcOnWire;
					canmsg->isExtended = can.status.extendedFrame;
					canmsg->isRemote = can.status.remoteFrame | can.status.canfdRTR;
					canmsg->isCANFD = can.status.canfdFDF;
					canmsg->baudrateSwitch = can.status.canfdBRS;
					canmsg->errorStateIndicator = can.status.canfdESI;
					return canmsg;
				}
				case Network::Type::Ethernet: {
					neomessage_eth_t& eth = *(neomessage_eth_t*)neomessage;
					auto ethmsg = std::make_shared<EthernetMessage>();
					ethmsg->network = network;
					ethmsg->description = eth.description;
					ethmsg->data.insert(ethmsg->data.end(), eth.data, eth.data + eth.length);
					return ethmsg;
				}
				default: break;
			}
			break;
		}
		default: break;
	}
	return std::shared_ptr<Message>();

}