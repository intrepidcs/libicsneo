#include "icsneo/communication/packet/linpacket.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/packetizer.h"

namespace icsneo {

std::shared_ptr<Message> HardwareLINPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareLINPacket* packet = reinterpret_cast<const HardwareLINPacket*>(bytestream.data());
	size_t numDataBytes = packet->CoreMiniBitsLIN.len;
	size_t numHeaderBytes = sizeof(HardwareLINPacket::CoreMiniBitsLIN);

	if( (sizeof(HardwareLINPacket) != bytestream.size()) ||
		((numDataBytes + numHeaderBytes) > bytestream.size()) )
		return nullptr;

	if(numDataBytes)
		--numDataBytes; //If data is present, there will be a checksum included

	auto msg = std::make_shared<LINMessage>(static_cast<uint8_t>(packet->CoreMiniBitsLIN.ID));
	msg->network = Network::GetNetIDFromCoreMiniNetwork(static_cast<Network::CoreMini>(packet->networkID));
	msg->isEnhancedChecksum = static_cast<bool>(packet->CoreMiniBitsLIN.TxChkSumEnhanced);

	/* Minimum one responder byte and one checksum byte. */
	if(2u > packet->CoreMiniBitsLIN.len)
		msg->linMsgType = LINMessage::Type::LIN_ERROR;

	auto dataStart = bytestream.begin() + numHeaderBytes;
	std::copy(dataStart, (dataStart+numDataBytes), std::back_inserter(msg->data));

	/* If OK, validate the checksum*/
	auto isChecksumInvalid = [&]() -> bool {
		/* messages with no data have no checksum (e.g. header only) */
		if(!msg->data.size())
			return true;

		uint8_t checkSum = (8 > numDataBytes) ? *(dataStart + numDataBytes) : packet->CoreMiniBitsLIN.LINByte9;
		LINMessage::calcChecksum(*msg);
		if(checkSum != msg->checksum) {
			msg->isEnhancedChecksum = true;
			LINMessage::calcChecksum(*msg);
			if(checkSum != msg->checksum) {
				msg->isEnhancedChecksum = false;
				msg->checksum = checkSum;
				return true;
			}
		}
		return false;
	};

	/* if any of the status bits are set, then this is 
	either a failed reception or a bus status update. */
	msg->errFlags =
	{
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxOnlyBreak),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxOnlyBreakSync),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrTxRxMismatch),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxBreakNotZero),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxBreakTooShort),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxSyncNot55),
		static_cast<bool>(packet->CoreMiniBitsLIN.ErrRxDataGreater8),
		static_cast<bool>(packet->CoreMiniBitsLIN.SyncFerr),
		static_cast<bool>(packet->CoreMiniBitsLIN.MidFerr),
		static_cast<bool>(packet->CoreMiniBitsLIN.ResponderByteFerr),
		isChecksumInvalid(), /* ErrChecksumMatch */
	};

	msg->statusFlags =
	{
		static_cast<bool>(packet->CoreMiniBitsLIN.TxChkSumEnhanced),
		static_cast<bool>(packet->CoreMiniBitsLIN.TXCommander),
		static_cast<bool>(packet->CoreMiniBitsLIN.TXResponder),
		static_cast<bool>(packet->CoreMiniBitsLIN.TxAborted),
		static_cast<bool>(packet->CoreMiniBitsLIN.UpdateResponderOnce),
		static_cast<bool>(packet->CoreMiniBitsLIN.HasUpdatedResponderOnce),
		static_cast<bool>(packet->CoreMiniBitsLIN.BusRecovered),
		static_cast<bool>(packet->CoreMiniBitsLIN.BreakOnly)
	};
	if(msg->statusFlags.TxCommander || msg->statusFlags.TxResponder)
		msg->linMsgType = LINMessage::Type::LIN_COMMANDER_MSG;
	else if(msg->statusFlags.BreakOnly)
		msg->linMsgType = LINMessage::Type::LIN_BREAK_ONLY;
	if( msg->errFlags.ErrRxBreakOnly     || msg->errFlags.ErrRxBreakSyncOnly ||
		msg->errFlags.ErrTxRxMismatch    || msg->errFlags.ErrRxBreakNotZero  ||
		msg->errFlags.ErrRxBreakTooShort || msg->errFlags.ErrRxSyncNot55     ||
		msg->errFlags.ErrRxDataLenOver8  || msg->errFlags.ErrFrameSync       ||
		msg->errFlags.ErrFrameMessageID  || msg->errFlags.ErrChecksumMatch   ||
		msg->errFlags.ErrFrameResponderData )
		{ msg->linMsgType = LINMessage::Type::LIN_ERROR; }

	msg->timestamp = packet->timestamp;
	return msg;
}

bool HardwareLINPacket::EncodeFromMessage(LINMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
{
	uint8_t size = ((std::min<size_t>(8ul, message.data.size()) + 3ul) & 0xFu);
	if(size > 3) { ++size; } // add a checksum byte if there's data
	switch(message.linMsgType) {
		case LINMessage::Type::LIN_HEADER_ONLY:
		case LINMessage::Type::LIN_COMMANDER_MSG:
		{
			size |= 0x80u;
			break;
		}
		case LINMessage::Type::LIN_BREAK_ONLY:
		{
			size |= 0x20u;
			break;
		}
		case LINMessage::Type::NOT_SET:
		{
			report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
			return false;
		}
		default:
			break;
	}
	message.calcProtectedID(message.ID);
	bytestream.insert(bytestream.end(),
	{
		static_cast<uint8_t>(0x00u),
		static_cast<uint8_t>(size),
		static_cast<uint8_t>((message.description >> 8) & 0xFF),
		static_cast<uint8_t>(message.description & 0xFF),
		static_cast<uint8_t>(message.protectedID)
	});

	switch(message.linMsgType) {
		case(LINMessage::Type::LIN_COMMANDER_MSG):
		case(LINMessage::Type::LIN_UPDATE_RESPONDER):
		{
			std::copy(message.data.begin(), message.data.end(), std::back_inserter(bytestream));
			LINMessage::calcChecksum(message);
			bytestream.push_back(message.checksum);
			break;
		}
		default:
			break;
	}

	if(bytestream.size() % 2)
		bytestream.push_back(0x41); //padding

	return true;
}

} //namespace icsneo