#include "icsneo/communication/packet/mdiopacket.h"

namespace icsneo
{

const size_t HardwareMDIOPacket::mdioDataSize = 2;

std::shared_ptr<Message> HardwareMDIOPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream)
{
	auto msg = std::make_shared<MDIOMessage>();
	const HardwareMDIOPacket* packet = reinterpret_cast<const HardwareMDIOPacket*>(bytestream.data());
	if((sizeof(HardwareMDIOPacket) != (bytestream.size())) || (packet->length != 0))
	{
		return nullptr; 
	}

	msg->network = Network::GetNetIDFromCoreMiniNetwork(static_cast<Network::CoreMini>(packet->networkID));
	msg->clause = static_cast<MDIOMessage::Clause>(packet->header.ST);
	msg->direction = static_cast<MDIOMessage::Direction>((packet->header.OP & 0x2) ? 1 : 0);
	msg->isTXMsg = static_cast<bool>(packet->header.TRANSMIT & 0x01u);
	msg->phyAddress = (packet->header.PHY_ADDR & 0x1Fu);
	if (msg->clause == MDIOMessage::Clause::Clause45)
	{ // 16-bit register address
		msg->devAddress = (packet->header.C45_DEVTYPE & 0x1Fu);
		msg->regAddress = (packet->header.REG_ADDR & 0xFFFFu);
	}
	else
	{ // 5-bit register address
		msg->devAddress = 0;
		msg->regAddress = (packet->header.REG_ADDR & 0x1Fu);
	}

	msg->isTXMsg = static_cast<bool>(packet->header.TRANSMIT & 0x01u);
	msg->txTimeout = static_cast<bool>(packet->header.ERR_TIMEOUT & 0x01u);
	msg->txAborted = static_cast<bool>(packet->header.ERR_JOB_CANCELLED & 0x01u);
	msg->txInvalidBus = static_cast<bool>(packet->header.ERR_INVALID_BUS & 0x01u);
	msg->txInvalidPhyAddr = static_cast<bool>(packet->header.ERR_INVALID_PHYADDR & 0x01u);
	msg->txInvalidRegAddr = static_cast<bool>(packet->header.ERR_INVALID_REGADDR & 0x01u);
	msg->txInvalidClause = static_cast<bool>(packet->header.ERR_UNSUPPORTED_CLAUSE & 0x01u);
	msg->txInvalidOpcode = static_cast<bool>(packet->header.ERR_UNSUPPORTED_OPCODE & 0x01u);
	//We don't care about 0xTRB0Dx in this case...
	//copy 0xTRB0STAT even though we likely won't use it either
	msg->description = packet->stats;
	msg->timestamp = (packet->timestamp & (0x7FFFFFFFFFFFFFFFull));

	std::copy(packet->data, packet->data + mdioDataSize, std::back_inserter(msg->data));

	return msg;
}

bool HardwareMDIOPacket::EncodeFromMessage(const MDIOMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
{
	const size_t numDataBytes = message.data.size();
	if(mdioDataSize < numDataBytes)
	{
		report(APIEvent::Type::MDIOMessageExceedsMaxLength, APIEvent::Severity::Error);
		return false;
	}

	uint8_t st = (message.clause == MDIOMessage::Clause::Clause45) ? 0x0 : 0x1;
	uint8_t op = (message.direction == MDIOMessage::Direction::Read) ? 0x2 : 0x1;
	uint8_t phyAddr = message.phyAddress & 0x1F;
	uint16_t regAddr = (message.clause == MDIOMessage::Clause::Clause45) ? message.regAddress : message.regAddress & 0x1F;
	uint8_t c45DevType = (message.clause == MDIOMessage::Clause::Clause45) ? message.devAddress & 0x1F : 0;

	bytestream.push_back(static_cast<uint8_t>((message.description >> 8) & 0xFF)); // MSB first
	bytestream.push_back(static_cast<uint8_t>(message.description & 0xFF)); // LSB 
	bytestream.push_back(op); // opcode
	bytestream.push_back(st); // st (clause)
	bytestream.push_back(phyAddr); // st (clause)
	bytestream.push_back(c45DevType); // clause 45 device type
	bytestream.push_back(regAddr & 0xFF); // reg addr LSB
	bytestream.push_back((regAddr >> 8) & 0xFF); // reg addr MSB

	std::copy(message.data.begin(), message.data.end(), std::back_inserter(bytestream));
	return true;
}

}