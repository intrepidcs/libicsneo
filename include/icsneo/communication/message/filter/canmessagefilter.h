#ifndef __CANMESSAGEFILTER_H_
#define __CANMESSAGEFILTER_H_

#ifdef __cplusplus

#include "icsneo/communication/message/filter/messagefilter.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include <memory>

namespace icsneo {

class CANMessageFilter : public MessageFilter {
public:
	CANMessageFilter() : MessageFilter(Network::Type::CAN), arbid(INVALID_ARBID) { messageType = Message::Type::BusMessage; }
	CANMessageFilter(uint32_t arbid) : MessageFilter(Network::Type::CAN), arbid(arbid) { messageType = Message::Type::BusMessage; }

	bool match(const std::shared_ptr<Message>& message) const {
		if(!MessageFilter::match(message))
			return false;
		const auto canMessage = std::dynamic_pointer_cast<CANMessage>(message);
		if(canMessage == nullptr || !matchArbID(canMessage->arbid))
			return false;
		return true;
	}

private:
	static constexpr uint32_t INVALID_ARBID = 0xffffffff;
	uint32_t arbid;
	bool matchArbID(uint32_t marbid) const {
		if(arbid == INVALID_ARBID)
			return true;
		return arbid == marbid;
	}
};

}

#endif // __cplusplus

#endif