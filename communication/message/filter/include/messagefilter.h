#ifndef __MESSAGEFILTER_H_
#define __MESSAGEFILTER_H_

#include "communication/include/network.h"
#include "communication/message/include/message.h"
#include <memory>

namespace icsneo {

class MessageFilter {
public:
	MessageFilter() {}
	MessageFilter(Network::Type type) : type(type) {}
	MessageFilter(Network::NetID netid) : netid(netid) {}
	virtual ~MessageFilter() {}
	// When getting "all" types of messages, include the ones marked as "internal only"
	bool includeInternalInAny = false;

	virtual bool match(const std::shared_ptr<Message>& message) const {
		if(!matchType(message->network.getType()))
			return false;
		if(!matchNetID(message->network.getNetID()))
			return false;
		return true;
	}

private:
	Network::Type type = Network::Type::Any;
	bool matchType(Network::Type mtype) const {
		if(type == Network::Type::Any && (mtype != Network::Type::Internal || includeInternalInAny))
			return true;
		return type == mtype;
	}

	Network::NetID netid = Network::NetID::Any;
	bool matchNetID(Network::NetID mnetid) const {
		if(netid == Network::NetID::Any)
			return true;
		return netid == mnetid;
	}
};

}

#endif