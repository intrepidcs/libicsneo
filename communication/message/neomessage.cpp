#include "communication/message/include/neomessage.h"
#include "communication/message/include/canmessage.h"

using namespace icsneo;

neomessage_t icsneo::CreateNeoMessage(const Message& message) {
	// This function is not responsible for storing the message!
	// Keep the shared_ptr around for the lifetime of the data access
	const auto type = message.network.getType();
	neomessage_t neomsg = { 0 }; // Clear out the memory
	neomsg.netid = (uint32_t)message.network.getNetID();
	neomsg.type = (uint8_t)type;
	neomsg.length = message.data.size();
	neomsg.data = message.data.data();
	neomsg.timestamp = message.timestamp;

	switch(type) {
		case Network::Type::CAN: {
			neomessage_can_t& can = *(neomessage_can_t*)&neomsg;
			const CANMessage& canmsg = *(const CANMessage*)&message;
			can.arbid = canmsg.arbid;
			can.status.extendedFrame = canmsg.isExtended;
			can.status.remoteFrame = canmsg.isRemote;
			can.status.canfdRTR = canmsg.isRemote;
			can.status.canfdFDF = canmsg.isCANFD;
			can.status.canfdBRS = canmsg.baudrateSwitch;
			break;
		}
		default:
			// TODO Implement others
			break;
	}

	return neomsg;
}