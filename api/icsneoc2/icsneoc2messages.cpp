#include "icsneo/icsneoc2messages.h" // TODO: Remove this after the complete refactor

#include "icsneo/icsneoc2.h"
#include "icsneoc2_internal.h"

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/packet/canpacket.h"

icsneoc2_error_t icsneoc2_message_is_valid(icsneoc2_message_t* message, bool* is_valid) {
	if(!message || !is_valid) {
		return icsneoc2_error_invalid_parameters;
	}

	*is_valid = (bool)message->message;

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_is_transmit(icsneoc2_message_t* message, bool* value) {
	if(!message || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	// We can static cast here because we are relying on the type being correct at this point
	auto frame = std::dynamic_pointer_cast<Frame>(message->message);
	if(!frame) {
		return icsneoc2_error_invalid_type;
	}
	*value = frame->transmitted;

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_netid_get(icsneoc2_message_t* message, icsneoc2_netid_t* netid) {
	if(!message || !netid) {
		return icsneoc2_error_invalid_parameters;
	}
	auto raw = std::dynamic_pointer_cast<RawMessage>(message->message);
	if(!raw) {
		return icsneoc2_error_invalid_type;
	}
	*netid = static_cast<icsneoc2_netid_t>(raw->network.getNetID());
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_netid_name_get(icsneoc2_netid_t netid, char* value, size_t* value_length) {
	if(!value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}
	auto netid_str = std::string(Network::GetNetIDString(static_cast<Network::NetID>(netid), true));
	// Copy the string into value
	return safe_str_copy(value, value_length, netid_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_message_netid_set(icsneoc2_message_t* message, icsneoc2_netid_t netid) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	auto raw = std::dynamic_pointer_cast<RawMessage>(message->message);
	if(!raw) {
		return icsneoc2_error_invalid_type;
	}
	raw->network = Network(static_cast<neonetid_t>(netid), true);
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_data_set(icsneoc2_message_t* message, uint8_t* data, size_t data_length) {
	if(!message || !data) {
		return icsneoc2_error_invalid_parameters;
	}
	// Make sure the message has the data field (RawMessage or Frame)
	auto raw_message = std::dynamic_pointer_cast<RawMessage>(message->message);
	if(!raw_message) {
		return icsneoc2_error_invalid_type;
	}
	raw_message->data.resize(data_length);
	std::copy(data, data + data_length, raw_message->data.begin());

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_data_get(icsneoc2_message_t* message, uint8_t* data, size_t* data_length) {
	if(!message || !data_length) {
		return icsneoc2_error_invalid_parameters;
	}
	// Make sure the message has the data field (RawMessage or Frame)
	auto raw_message = std::dynamic_pointer_cast<RawMessage>(message->message);
	if(!raw_message) {
		return icsneoc2_error_invalid_type;
	}
	if(!data) {
		*data_length = raw_message->data.size();
		return icsneoc2_error_success;
	}
	if(*data_length < raw_message->data.size()) {
		return icsneoc2_error_invalid_parameters;
	}
	std::copy(raw_message->data.begin(), raw_message->data.begin() + raw_message->data.size(), data);
	*data_length = raw_message->data.size();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_can_create(icsneoc2_message_t** message) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	try {
		*message = new icsneoc2_message_t;
		// Initialize the internal message as a CANMessage so that all icsneoc2_message_can_*_set
		// functions work correctly on user-created transmit messages.
		(*message)->message = std::make_shared<CANMessage>();
	} catch(const std::bad_alloc&) {
		return icsneoc2_error_out_of_memory;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_free(icsneoc2_message_t* message) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	delete message;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_can_props_set(icsneoc2_message_t* message, const uint64_t* arb_id, const uint64_t* flags) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	auto can_msg = std::dynamic_pointer_cast<CANMessage>(message->message);
	if(!can_msg) {
		return icsneoc2_error_invalid_type;
	}
	if(arb_id) {
		can_msg->arbid = static_cast<uint32_t>(*arb_id);
	}
	if(flags) {
		can_msg->isRemote = (*flags & ICSNEOC2_MESSAGE_CAN_FLAGS_RTR);
		can_msg->isExtended = (*flags & ICSNEOC2_MESSAGE_CAN_FLAGS_IDE);
		can_msg->isCANFD = (*flags & ICSNEOC2_MESSAGE_CAN_FLAGS_FDF);
		can_msg->baudrateSwitch = (*flags & ICSNEOC2_MESSAGE_CAN_FLAGS_BRS);
		can_msg->errorStateIndicator = (*flags & ICSNEOC2_MESSAGE_CAN_FLAGS_ESI);
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_can_props_get(icsneoc2_message_t* message, uint64_t* arb_id, uint64_t* flags) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	auto can_msg = std::dynamic_pointer_cast<CANMessage>(message->message);
	if(!can_msg) {
		return icsneoc2_error_invalid_type;
	}
	if(arb_id) {
		*arb_id = can_msg->arbid;
	}
	if(flags) {
		*flags = 0;
		if(can_msg->isRemote) {
			*flags |= ICSNEOC2_MESSAGE_CAN_FLAGS_RTR;
		}
		if(can_msg->isExtended) {
			*flags |= ICSNEOC2_MESSAGE_CAN_FLAGS_IDE;
		}
		if(can_msg->isCANFD) {
			*flags |= ICSNEOC2_MESSAGE_CAN_FLAGS_FDF;
		}
		if(can_msg->baudrateSwitch) {
			*flags |= ICSNEOC2_MESSAGE_CAN_FLAGS_BRS;
		}
		if(can_msg->errorStateIndicator) {
			*flags |= ICSNEOC2_MESSAGE_CAN_FLAGS_ESI;
		}
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_network_type_get(icsneoc2_message_t* message, icsneoc2_network_type_t* network_type) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	auto raw = std::dynamic_pointer_cast<RawMessage>(message->message);
	if(!raw) {
		return icsneoc2_error_invalid_type;
	}
	*network_type = (icsneoc2_network_type_t)raw->network.getType();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_is_raw(icsneoc2_message_t* message, bool* is_raw) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	*is_raw = std::dynamic_pointer_cast<RawMessage>(message->message) != nullptr;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_is_frame(icsneoc2_message_t* message, bool* is_frame) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	*is_frame = std::dynamic_pointer_cast<Frame>(message->message) != nullptr;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_message_is_can(icsneoc2_message_t* message, bool* is_can) {
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	*is_can = std::dynamic_pointer_cast<CANMessage>(message->message) != nullptr;
	return icsneoc2_error_success;

}
