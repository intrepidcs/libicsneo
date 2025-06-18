#include "icsneo/communication/packet/livedatapacket.h"
#include "icsneo/communication/message/livedatamessage.h"
#include <cstring>
#include <vector>

namespace icsneo {

std::shared_ptr<Message> HardwareLiveDataPacket::DecodeToMessage(const std::vector<uint8_t>& bytes, const device_eventhandler_t& report) {
	if(bytes.empty() || (bytes.size() < (sizeof(LiveDataHeader) + sizeof(ExtResponseHeader)))) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return nullptr;
	}

	const auto header = reinterpret_cast<const ExtResponseHeader*>(bytes.data());
	if(ExtendedCommand::LiveData != static_cast<ExtendedCommand>(header->command)) {
		report(APIEvent::Type::LiveDataInvalidCommand, APIEvent::Severity::Error);
		return nullptr;
	}

	const auto ldHeader = reinterpret_cast<const LiveDataHeader*>(bytes.data() + sizeof(ExtResponseHeader));
	// Versioning check to avoid bad data interpretation between disparate libicsneo and firmware versions
	if(icsneo::LiveDataUtil::LiveDataVersion != ldHeader->version) {
		report(APIEvent::Type::LiveDataVersionMismatch, APIEvent::Severity::Error);
		return nullptr;
	}
	switch(LiveDataCommand(ldHeader->cmd)) {
		case LiveDataCommand::RESPONSE: {
			auto retMsg = std::make_shared<LiveDataValueMessage>();
			const auto responseBytes = reinterpret_cast<const LiveDataValueResponse*>(ldHeader);
			retMsg->handle = responseBytes->handle;
			retMsg->cmd = static_cast<LiveDataCommand>(responseBytes->cmd);
			retMsg->numArgs = responseBytes->numArgs;
			for(uint32_t i = 0; i < retMsg->numArgs; ++i) {
				retMsg->values.emplace_back(std::make_shared<LiveDataValue>(responseBytes->values[i]));
			}
			return retMsg;
		}
		case LiveDataCommand::STATUS: {
			auto retMsg = std::make_shared<LiveDataStatusMessage>();
			const auto responseBytes = reinterpret_cast<const LiveDataStatusResponse*>(ldHeader);
			retMsg->handle = responseBytes->handle;
			retMsg->cmd = static_cast<LiveDataCommand>(responseBytes->cmd);
			retMsg->status = responseBytes->status;
			retMsg->requestedCommand = static_cast<LiveDataCommand>(responseBytes->requestedCommand);
			return retMsg;
		}
		default: {
			report(APIEvent::Type::LiveDataInvalidCommand, APIEvent::Severity::Error);
			break;
		}
	}
	return nullptr;
}

bool HardwareLiveDataPacket::EncodeFromMessage(LiveDataMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	uint16_t payloadSize = 0;
	switch(message.cmd) {
		case LiveDataCommand::SUBSCRIBE: {
			auto commandMsg = reinterpret_cast<LiveDataCommandMessage*>(&message);
			const auto numArgs = commandMsg->args.size();
			if(numArgs) {
				payloadSize = static_cast<uint16_t>(sizeof(LiveDataSubscribe) + (sizeof(LiveDataArgument) * (numArgs-1)));
				bytestream.resize((payloadSize + sizeof(ExtendedCommandHeader)),0);
				LiveDataSubscribe* out = reinterpret_cast<LiveDataSubscribe*>(bytestream.data() + sizeof(ExtendedCommandHeader));
				out->version = icsneo::LiveDataUtil::LiveDataVersion;
				out->cmd = static_cast<uint32_t>(commandMsg->cmd);
				if(!commandMsg->handle)
					commandMsg->handle = LiveDataUtil::getNewHandle();
				out->handle = commandMsg->handle;
				out->numArgs = static_cast<uint32_t>(commandMsg->args.size());
				out->freqMs = static_cast<uint32_t>(commandMsg->updatePeriod.count());
				out->expireMs = static_cast<uint32_t>(commandMsg->expirationTime.count());
				for(size_t i = 0; i < numArgs; ++i) {
					out->args[i].objectType = commandMsg->args[i]->objectType;
					out->args[i].objectIndex = commandMsg->args[i]->objectIndex;
					out->args[i].signalIndex = commandMsg->args[i]->signalIndex;
					out->args[i].valueType = commandMsg->args[i]->valueType;
				}
			} else {
				report(APIEvent::Type::LiveDataInvalidArgument, APIEvent::Severity::Error);
				return false;
			}
			break;
		}
		case LiveDataCommand::UNSUBSCRIBE: {
			payloadSize = sizeof(LiveDataHeader);
			bytestream.resize((payloadSize + sizeof(ExtendedCommandHeader)),0);
			auto ldUnsubMsg = reinterpret_cast<LiveDataHeader*>(bytestream.data() + sizeof(ExtendedCommandHeader));
			ldUnsubMsg->version = static_cast<uint32_t>(icsneo::LiveDataUtil::LiveDataVersion);
			ldUnsubMsg->cmd = static_cast<uint32_t>(message.cmd);
			ldUnsubMsg->handle = static_cast<uint32_t>(message.handle);
			break;
		}
		case LiveDataCommand::CLEAR_ALL: {
			payloadSize = sizeof(LiveDataHeader);
			bytestream.resize((payloadSize + sizeof(ExtendedCommandHeader)),0);
			auto clearMsg = reinterpret_cast<LiveDataHeader*>(bytestream.data() + sizeof(ExtendedCommandHeader));
			clearMsg->version = static_cast<uint32_t>(icsneo::LiveDataUtil::LiveDataVersion);
			clearMsg->cmd = static_cast<uint32_t>(message.cmd);
			break;
		}
		case LiveDataCommand::SET_VALUE: {
			auto setValMsg = reinterpret_cast<LiveDataSetValueMessage*>(&message);
			const auto numArgs = setValMsg->args.size();
			if(numArgs) {
				payloadSize = static_cast<uint16_t>(sizeof(LiveDataSetValue) + (sizeof(LiveDataSetValueEntry) * (numArgs-1)));
				bytestream.resize((payloadSize + sizeof(ExtendedCommandHeader)),0);
				LiveDataSetValue* out = reinterpret_cast<LiveDataSetValue*>(bytestream.data() + sizeof(ExtendedCommandHeader));
				out->version = icsneo::LiveDataUtil::LiveDataVersion;
				out->cmd = static_cast<uint32_t>(setValMsg->cmd);
				if(!setValMsg->handle)
					setValMsg->handle = LiveDataUtil::getNewHandle();
				out->handle = setValMsg->handle;
				out->numSetValues = (uint32_t)numArgs;
				for(size_t i = 0; i < numArgs; ++i) {
					out->values[i].arg.objectType = setValMsg->args[i]->objectType;
					out->values[i].arg.objectIndex = setValMsg->args[i]->objectIndex;
					out->values[i].arg.signalIndex = setValMsg->args[i]->signalIndex;
					out->values[i].arg.valueType = setValMsg->args[i]->valueType;
					out->values[i].value.value = setValMsg->values[i]->value;
					out->values[i].value.header.length = sizeof(LiveDataValue::value);
				}
			} else {
				report(APIEvent::Type::LiveDataInvalidArgument, APIEvent::Severity::Error);
				return false;
			}
			break;
		}
		default: {
			report(APIEvent::Type::LiveDataInvalidCommand, APIEvent::Severity::Error);
			return false;
		}
	}
	// +1 for AA, another +1 for firmware nuance
	uint16_t fullSize = static_cast<uint16_t>(1 + sizeof(ExtendedCommandHeader) + payloadSize) + 1;

	ExtendedCommandHeader* header = reinterpret_cast<ExtendedCommandHeader*>(bytestream.data());
	if(!header) {
		report(APIEvent::Type::LiveDataEncoderError, APIEvent::Severity::Error);
		return false;
	}

	header->netid = static_cast<uint8_t>(Network::NetID::Main51);
	header->fullLength = fullSize;
	header->command = static_cast<uint8_t>(Command::Extended);
	header->extendedCommand = static_cast<uint16_t>(ExtendedCommand::LiveData);
	header->payloadLength = payloadSize;

	return true;
}

} // namespace icsneo
