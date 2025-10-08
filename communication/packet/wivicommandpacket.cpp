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
			if(bytestream.size() < sizeof(WiVI::CommandPacket::GetAllHeader))
				return {};

			const auto& getAll = *reinterpret_cast<const WiVI::CommandPacket::GetAllHeader*>(bytestream.data());
			msg->responseTo = WiVI::Command::GetAll;
			msg->info.emplace();
			msg->info->sleepRequest = getAll.sleepRequest;
			msg->info->connectionTimeoutMinutes = getAll.connectionTimeoutMinutes;

			// Check that we have enough data for the capture infos
			size_t captureInfosSize = sizeof(WiVI::CaptureInfo) * getAll.numCaptureInfos;
			if(bytestream.size() < sizeof(WiVI::CommandPacket::GetAllHeader) + captureInfosSize)
				return {};

			const WiVI::CaptureInfo* const captureInfos = (const WiVI::CaptureInfo*)(bytestream.data() + sizeof(WiVI::CommandPacket::GetAllHeader));
			msg->info->captures.resize(getAll.numCaptureInfos);
			for(uint16_t i = 0; i < getAll.numCaptureInfos; i++)
				msg->info->captures[i] = captureInfos[i];

			
			// New field vinAvail was added - check if it is present:
			if(bytestream.size() >= sizeof(WiVI::CommandPacket::GetAllHeader) + captureInfosSize + 2) {
				msg->info->vinAvailable = *(bytestream.data() + sizeof(WiVI::CommandPacket::GetAllHeader) + captureInfosSize);
			} else {
				msg->info->vinAvailable = 0;
			}
				
			break;
		}
		case WiVI::Command::GetVIN: {
			if (bytestream.size() < sizeof(WiVI::CommandPacket::GetVIN))
				return {};

			const auto& getVIN = *reinterpret_cast<const WiVI::CommandPacket::GetVIN*>(bytestream.data());
			msg->responseTo = WiVI::Command::GetVIN;
			msg->vin  = getVIN.VIN;

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

std::vector<uint8_t> WiVI::CommandPacket::GetAllHeader::Encode() {
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::GetAllHeader));
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::GetAllHeader*>(ret.data());

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

std::vector<uint8_t> WiVI::CommandPacket::GetVIN::Encode()
{
	std::vector<uint8_t> ret(sizeof(WiVI::CommandPacket::GetVIN));
	auto& frame = *reinterpret_cast<WiVI::CommandPacket::GetVIN*>(ret.data());

	frame.header.cmd = WiVI::Command::GetVIN;
	frame.header.length = sizeof(frame) - sizeof(frame.header);

	return ret;
}

