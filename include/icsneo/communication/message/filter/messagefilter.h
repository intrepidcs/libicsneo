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
	MessageFilter(_icsneo_netid_t netid) : MessageFilter(Network::GetTypeOfNetID(netid, false), netid) {} // Messages on the communication layer are never encoded as VNET ID + common ID, so skip the expansion step
	MessageFilter(icsneo_msg_bus_type_t type, _icsneo_netid_t net = _icsneo_netid_t::icsneo_netid_any) : networkType(type), netid(net) {
		// If a icsneo_msg_bus_type_internal is used, we want to also get internal Message::Types
		// The NetID we want may be in there
		includeInternalInAny = (networkType == icsneo_msg_bus_type_internal);
	}
	virtual ~MessageFilter() = default;
	// When getting "all" types of messages, include the ones marked as "internal only"
	bool includeInternalInAny = false;

	virtual bool match(const std::shared_ptr<Message>& message) const {
		if(!matchMessageType(message->type))
			return false;

		if(message->type == Message::Type::BusMessage || message->type == Message::Type::Main51 || 
			message->type == Message::Type::InternalMessage || message->type == Message::Type::ReadSettings) {
			const auto frame = std::static_pointer_cast<InternalMessage>(message);
			if(!matchNetworkType(frame->network.getType()))
				return false;
			if(!matchNetID(frame->network.getNetID()))
				return false;
		} else if (netid != _icsneo_netid_t::icsneo_netid_any || networkType != icsneo_msg_bus_type_any) {
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

	icsneo_msg_bus_type_t networkType = icsneo_msg_bus_type_any;
	bool matchNetworkType(icsneo_msg_bus_type_t mtype) const {
		if(networkType == icsneo_msg_bus_type_any && (mtype != icsneo_msg_bus_type_internal || includeInternalInAny))
			return true;
		return networkType == mtype;
	}

	_icsneo_netid_t netid = _icsneo_netid_t::icsneo_netid_any;
	bool matchNetID(_icsneo_netid_t mnetid) const {
		if(netid == _icsneo_netid_t::icsneo_netid_any)
			return true;
		return netid == mnetid;
	}
};

}

#endif // __cplusplus

#endif