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
	MessageFilter(icsneo_msg_bus_type_t type, Network::NetID net = Network::NetID::Any) : networkType(type), netid(net) {
		// If a Network::Type::Internal is used, we want to also get internal Message::Types
		// The NetID we want may be in there
		includeInternalInAny = (networkType == _icsneo_msg_bus_type_t::icsneo_msg_bus_type_internal);
	}
	virtual ~MessageFilter() = default;
	// When getting "all" types of messages, include the ones marked as "internal only"
	bool includeInternalInAny = false;

	virtual bool match(const std::shared_ptr<Message>& message) const {
		if(!matchMessageType(message->type))
			return false;

		if(message->type == Message::Type::Frame || message->type == Message::Type::Main51 || 
			message->type == Message::Type::RawMessage || message->type == Message::Type::ReadSettings) {
			const auto frame = std::static_pointer_cast<InternalMessage>(message);
			if(!matchNetworkType(frame->network.getType()))
				return false;
			if(!matchNetID(frame->network.getNetID()))
				return false;
		} else if (netid != Network::NetID::Any || networkType != _icsneo_msg_bus_type_t::icsneo_msg_bus_type_any) {
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

	icsneo_msg_bus_type_t networkType = _icsneo_msg_bus_type_t::icsneo_msg_bus_type_any;
	bool matchNetworkType(icsneo_msg_bus_type_t mtype) const {
		if(networkType == _icsneo_msg_bus_type_t::icsneo_msg_bus_type_any && (mtype != _icsneo_msg_bus_type_t::icsneo_msg_bus_type_internal || includeInternalInAny))
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