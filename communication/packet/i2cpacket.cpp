#include "icsneo/communication/packet/i2cpacket.h"

namespace icsneo
{
	std::shared_ptr<Message> HardwareI2CPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream)
	{
		auto msg = std::make_shared<I2CMessage>();
		const I2CHeader* packet = reinterpret_cast<const I2CHeader*>(bytestream.data());
		const size_t numPayloadBytes = packet->length;
		const size_t numControlBytes = packet->CoreMiniBitsI2C.CBLen;
		const size_t numDataBytes = numPayloadBytes - numControlBytes;
		if( (numPayloadBytes == 0) || (numDataBytes > I2CMaxLength) ||
			(sizeof(I2CHeader) != (bytestream.size() - numPayloadBytes)) )
		{ return nullptr; }

		msg->network = Network::GetNetIDFromCoreMiniNetwork(static_cast<Network::CoreMini>(packet->networkID));
		msg->address = (packet->CoreMiniBitsI2C.ID & 0x3FFu);
		msg->deviceMode = static_cast<I2CMessage::DeviceMode>(packet->CoreMiniBitsI2C.CT);
		msg->direction = static_cast<I2CMessage::Direction>(packet->CoreMiniBitsI2C.DIR);

		msg->isExtendedID = static_cast<bool>(packet->CoreMiniBitsI2C.EID & 0x01u);
		msg->isTXMsg = static_cast<bool>(packet->CoreMiniBitsI2C.TXMsg & 0x01u);
		msg->txTimeout = static_cast<bool>(packet->CoreMiniBitsI2C.TXTimeout & 0x01u);
		msg->txNack = static_cast<bool>(packet->CoreMiniBitsI2C.TXNack & 0x01u);
		msg->txAborted = static_cast<bool>(packet->CoreMiniBitsI2C.TXAborted & 0x01u);
		msg->txLostArb = static_cast<bool>(packet->CoreMiniBitsI2C.TXLostArb & 0x01u);
		msg->txError = static_cast<bool>(packet->CoreMiniBitsI2C.TXError & 0x01u);
		//We don't care about 0xTRB0Dx in this case...
		//copy 0xTRB0STAT even though we likely won't use it either
		msg->stats = packet->stats;
		msg->timestamp = (packet->timestamp & (0x7FFFFFFFFFFFFFFFull));

		//The device will combine the 'control' bytes and data bytes into one payload
		//The control bytes will always come before the data
		auto cbStart = bytestream.begin() + sizeof(I2CHeader);
		auto dataStart = cbStart + numControlBytes;
		std::copy(cbStart, dataStart, std::back_inserter(msg->controlBytes));
		std::copy(dataStart, bytestream.end(), std::back_inserter(msg->dataBytes));

		return msg;
	}

	bool HardwareI2CPacket::EncodeFromMessage(const I2CMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
	{
		const size_t numControlBytes = message.controlBytes.size();
		const size_t numDataBytes = message.dataBytes.size();
		if(I2CMaxLength < numDataBytes)
		{
			report(APIEvent::Type::I2CMessageExceedsMaxLength, APIEvent::Severity::Error);
			return false;
		}
		if(message.controlBytes.empty() && message.dataBytes.empty())
		{
			//You'll need to provide a target R/W register in controlBytes
			//alternatively, you're expecting to read without providing a dataBytes payload
			report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
			return false;
		}

		bytestream.push_back(static_cast<uint8_t>(numControlBytes & 0xFFu));
		bytestream.push_back(static_cast<uint8_t>((numControlBytes) >> 8) & 0xFFu);
		bytestream.push_back(static_cast<uint8_t>(numDataBytes & 0xFFu));
		bytestream.push_back(static_cast<uint8_t>((numDataBytes) >> 8) & 0xFFu);
		bytestream.push_back(static_cast<uint8_t>((message.stats) >> 8) & 0xFFu);
		bytestream.push_back(static_cast<uint8_t>(message.stats & 0xFFu));

		if(message.isExtendedID)
		{
			bytestream.push_back(static_cast<uint8_t>(message.address & 0xFFu));
			bytestream.push_back(static_cast<uint8_t>(((message.address) >> 8) & 0x03u) | 0x04u);
		} else {
			bytestream.push_back(static_cast<uint8_t>(message.address & 0xFFu));
			bytestream.push_back(static_cast<uint8_t>(0x00u));
		}

		if(I2CMessage::Direction::Read == message.direction)
		{ bytestream.back() |= static_cast<uint8_t>(0x10u); }

		std::copy(message.controlBytes.begin(), message.controlBytes.end(), std::back_inserter(bytestream));
		std::copy(message.dataBytes.begin(), message.dataBytes.end(), std::back_inserter(bytestream));
		return true;
	}
}