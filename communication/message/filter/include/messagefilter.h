#ifndef __MESSAGEFILTER_H_
#define __MESSAGEFILTER_H_

#include "communication/include/network.h"
#include "communication/message/include/message.h"
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

};

#endif