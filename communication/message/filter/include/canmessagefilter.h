#ifndef __CANMESSAGEFILTER_H_
#define __CANMESSAGEFILTER_H_

#include "communication/message/filter/include/messagefilter.h"
#include "communication/include/network.h"
#include "communication/message/include/message.h"
#include "communication/message/include/canmessage.h"
#include <memory>

namespace icsneo {

class CANMessageFilter : public MessageFilter {
public:
	CANMessageFilter() : MessageFilter(Network::Type::CAN), arbid(INVALID_ARBID) {}
	CANMessageFilter(uint32_t arbid) : MessageFilter(Network::Type::CAN), arbid(arbid) {}

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

};

#endif