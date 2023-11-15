#ifndef __MESSAGEFILTER_H_
#define __MESSAGEFILTER_H_

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include "icsneo/communication/message/message.h"
#include <memory>

namespace icsneo {

class MessageFilter {
public:
	MessageFilter() {}
	MessageFilter(Message::Type type) : includeInternalInAny(neomessagetype_t(type) & 0x8000), messageType(type) {}
	MessageFilter(Network::NetID netid) : MessageFilter(Network::GetTypeOfNetID(netid, false), netid) {} // Messages on the communication layer are never encoded as VNET ID + common ID, so skip the expansion step
	MessageFilter(Network::Type type, Network::NetID net = Network::NetID::Any) : networkType(type), netid(net) {
		// If a Network::Type::Internal is used, we want to also get internal Message::Types
		// The NetID we want may be in there
		includeInternalInAny = (networkType == Network::Type::Internal);
	}
	virtual ~MessageFilter() = default;
	// When getting "all" types of messages, include the ones marked as "internal only"
	bool includeInternalInAny = false;

	virtual bool match(const std::shared_ptr<Message>& message) const {
		if(!matchMessageType(message->type))
			return false;

		if(message->type == Message::Type::Frame || message->type == Message::Type::Main51 || 
			message->type == Message::Type::RawMessage || message->type == Message::Type::ReadSettings) {
			const auto frame = std::dynamic_pointer_cast<RawMessage>(message);
			if(!matchNetworkType(frame->network.getType()))
				return false;
			if(!matchNetID(frame->network.getNetID()))
				return false;
		} else if (netid != Network::NetID::Any || networkType != Network::Type::Any) {
			return false; // Filtering on a NetID or Type, but this message doesn't have one
		}
		return true;
	}

protected:
	Message::Type messageType = Message::Type::Invalid; // Used here for "any"
	bool matchMessageType(Message::Type mtype) const {
		if(messageType == Message::Type::Invalid && ((neomessagetype_t(mtype) & 0x8000) == 0 || includeInternalInAny))
			return true;
		return messageType == mtype;
	}

	Network::Type networkType = Network::Type::Any;
	bool matchNetworkType(Network::Type mtype) const {
		if(networkType == Network::Type::Any && (mtype != Network::Type::Internal || includeInternalInAny))
			return true;
		return networkType == mtype;
	}

	Network::NetID netid = Network::NetID::Any;
	bool matchNetID(Network::NetID mnetid) const {
		if(netid == Network::NetID::Any)
			return true;
		return netid == mnetid;
	}
};

}

#endif // __cplusplus

#endif