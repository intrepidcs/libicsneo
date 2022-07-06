#include "icsneo/communication/packet/wivicommandpacket.h"
#include "icsneo/communication/message/wiviresponsemessage.h"
#include <cstring>

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

			const auto& decoded = *reinterpret_cast<const WiVI::CommandPacket::Result*>(bytestream.data());
			msg->responseTo = decoded.responseTo;
			msg->success = decoded.result != 0;
			break;
		}
		case WiVI::Command::GetSignal: {
			// Use the SetSignal structure since it matches the response
			if(bytestream.size() < sizeof(WiVI::CommandPacket::SetSignal))
				return {};

			const auto& setSignal = *reinterpret_cast<const WiVI::CommandPacket::SetSignal*>(bytestream.data());
			msg->responseTo = WiVI::Command::GetSignal;
			msg->value = setSignal.value.ValueInt32;
			break;
		}
		case WiVI::Command::GetAll: {
			if(bytestream.size() < sizeof(WiVI::CommandPacket::GetAll))
				return {};

			const auto& getAll = *reinterpret_cast<const WiVI::CommandPacket::GetAll*>(bytestream.data());
			msg->responseTo = WiVI::Command::GetAll;
			msg->info.emplace();
			msg->info->sleepRequest = getAll.sleepRequest;
			msg->info->connectionTimeoutMinutes = getAll.connectionTimeoutMinutes;

			// Check that we have enough data for the capture infos
			if(bytestream.size() < sizeof(WiVI::CommandPacket::GetAll) + (sizeof(WiVI::CaptureInfo) * getAll.numCaptureInfos))
				return {};

			msg->info->captures.resize(getAll.numCaptureInfos);
			for(uint16_t i = 0; i < getAll.numCaptureInfos; i++)
				msg->info->captures[i] = getAll.captureInfos[i];
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

std::vector<uint8_t> WiVI::CommandPacket::GetAll::Encode() {
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::GetAll));
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::GetAll*>(ret.data());

	frame.header.cmd = WiVI::Command::GetAll;
	frame.header.length = sizeof(frame) - sizeof(frame.header);

	return ret;
}

std::vector<uint8_t> WiVI::CommandPacket::ClearUploads::Encode(const std::vector<uint8_t>& bitmask) {
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::ClearUploads) + bitmask.size());
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::ClearUploads*>(ret.data());

	frame.header.cmd = WiVI::Command::ClearUploads;
	frame.header.length = uint16_t(ret.size() - sizeof(frame.header));
	memcpy(frame.bitmask, bitmask.data(), bitmask.size());

	return ret;
}
