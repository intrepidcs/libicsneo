#include "icsneo/communication/packet/wivicommandpacket.h"
#include "icsneo/communication/message/wiviresponsemessage.h"

using namespace icsneo;

std::shared_ptr<WiVI::ResponseMessage> WiVI::CommandPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(WiVI::CommandPacket::Header))
		return {};

	auto msg = std::make_shared<WiVI::ResponseMessage>();
	const auto& header = *reinterpret_cast<const WiVI::CommandPacket::Header*>(bytestream.data());
	switch(header.cmd) {
		case WiVI::Command::Result: {
			if(bytestream.size() < sizeof(WiVI::CommandPacket::Result))
				return {};

			if(bytestream.size() != sizeof(WiVI::CommandPacket::Header) + header.length)
				return {};

			const auto& decoded = *reinterpret_cast<const WiVI::CommandPacket::Result*>(bytestream.data());
			msg->responseTo = decoded.responseTo;
			msg->success = decoded.result != 0;
			break;
		}
		case WiVI::Command::GetSignal: {
			// Use the SetSignal structure since it matches the response
			if(bytestream.size() < sizeof(WiVI::CommandPacket::SetSignal))
				return {};

			if(bytestream.size() != sizeof(WiVI::CommandPacket::SetSignal) + header.length)
				return {};

			const auto& setSignal = *reinterpret_cast<const WiVI::CommandPacket::SetSignal*>(bytestream.data());
			msg->responseTo = WiVI::Command::GetSignal;
			msg->value = setSignal.value.ValueInt32;
			break;
		}
		default: // Unknown command response
			return {};
	}
	return msg;
}

std::vector<uint8_t> WiVI::CommandPacket::GetSignal::Encode(WiVI::SignalType type) {
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::GetSignal));
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::GetSignal*>(ret.data());

	frame.header.cmd = WiVI::Command::GetSignal;
	frame.header.length = sizeof(frame) - sizeof(frame.header);
	frame.type = type;

	return ret;
}

std::vector<uint8_t> WiVI::CommandPacket::SetSignal::Encode(WiVI::SignalType type, CoreMiniFixedPointValue value) {
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::SetSignal));
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::SetSignal*>(ret.data());

	frame.header.cmd = WiVI::Command::SetSignal;
	frame.header.length = sizeof(frame) - sizeof(frame.header);
	frame.type = type;
	frame.value = value;

	return ret;
}
