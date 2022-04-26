#include "icsneo/communication/message/ethphymessage.h"

namespace icsneo
{
bool EthPhyMessage::appendPhyMessage(bool writeEnable, bool clause45, uint8_t phyAddrOrPort, uint8_t pageOrDevice, uint16_t regAddr, uint16_t regVal, bool enabled)
{
	auto msg = std::make_shared<PhyMessage>();
	msg->Clause45Enable = clause45;
	msg->Enabled = enabled;
	msg->WriteEnable = writeEnable;
	msg->version = 1u;
	if( (FiveBits < phyAddrOrPort) ||
		(clause45 && (FiveBits < pageOrDevice)) ||
		(!clause45 && (FiveBits < regAddr)) )
	{
		return false;
	}

	if(clause45)
	{
		msg->clause45.port    = phyAddrOrPort;
		msg->clause45.device  = pageOrDevice;
		msg->clause45.regAddr = regAddr;
		msg->clause45.regVal  = regVal;
	}
	else
	{
		msg->clause22.phyAddr = phyAddrOrPort;
		msg->clause22.page    = pageOrDevice;
		msg->clause22.regAddr = regAddr;
		msg->clause22.regVal  = regVal;
	}
	return appendPhyMessage(msg);
}

bool EthPhyMessage::appendPhyMessage(std::shared_ptr<PhyMessage> message)
{
	if(message != nullptr)
	{
		messages.push_back(message);
		return true;
	}
	return false;
}

size_t EthPhyMessage::getMessageCount() const
{
	return messages.size();
}
}
