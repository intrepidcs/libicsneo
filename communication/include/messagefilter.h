#ifndef __MESSAGEFILTER_H_
#define __MESSAGEFILTER_H_

#include "communication/include/network.h"
#include "communication/message/include/message.h"
#include "communication/message/include/canmessage.h"
#include <memory>

namespace icsneo {

class MessageFilter {
public:
	MessageFilter() : matchAny(true) {}
	MessageFilter(Network::Type type) : type(type) {}
	MessageFilter(Network::NetID netid) : netid(netid) {}
	virtual ~MessageFilter() {}

	virtual bool match(const std::shared_ptr<Message>& message) const {
		if(matchAny)
			return true;
		if(!matchType(message->network.getType()))
			return false;
		if(!matchNetID(message->network.getNetID()))
			return false;
		return true;
	}

private:
	bool matchAny = false;

	Network::Type type = Network::Type::Invalid; // Matching a type of invalid will match any
	bool matchType(Network::Type mtype) const {
		if(type == Network::Type::Invalid)
			return true;
		return type == mtype;
	}

	Network::NetID netid = Network::NetID::Invalid; // Matching a netid of invalid will match any
	bool matchNetID(Network::NetID mnetid) const {
		if(netid == Network::NetID::Invalid)
			return true;
		return netid == mnetid;
	}
};

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

}

#endif