#include "icsneo/communication/packet/ethphyregpacket.h"
#include "icsneo/communication/message/ethphymessage.h"
#include "icsneo/communication/packetizer.h"
#include <memory>
#include <cstdint>
#include <iostream>

namespace icsneo
{
std::shared_ptr<EthPhyMessage> HardwareEthernetPhyRegisterPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
{
	if(bytestream.empty() || (bytestream.size() < sizeof(PhyRegisterHeader_t)))
	{
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return nullptr;
	}
	auto msg = std::make_shared<EthPhyMessage>();
	const PhyRegisterHeader_t* pHeader = reinterpret_cast<const PhyRegisterHeader_t*>(bytestream.data());
	const size_t numEntries = static_cast<size_t>(pHeader->numEntries);
	if(
		(PhyPacketVersion == pHeader->version) &&
		(sizeof(PhyRegisterPacket_t) == pHeader->entryBytes) &&
		(numEntries <= MaxPhyEntries) &&
		((bytestream.size() - sizeof(PhyRegisterHeader_t))
			== (sizeof(PhyRegisterPacket_t) * numEntries))
	)
	{
		msg->messages.reserve(numEntries);
		const PhyRegisterPacket_t* pFirstEntry = reinterpret_cast<const PhyRegisterPacket_t*>(bytestream.data() + sizeof(PhyRegisterHeader_t));
		for(size_t entryIdx{0}; entryIdx < numEntries; ++entryIdx)
		{
			const PhyRegisterPacket_t* pEntry = (pFirstEntry + entryIdx);
			auto phyMessage = std::make_shared<PhyMessage>();
			phyMessage->Enabled = (pEntry->Enabled != 0u);
			phyMessage->WriteEnable = (pEntry->WriteEnable != 0u);
			phyMessage->Clause45Enable = (pEntry->Clause45Enable != 0u);
			phyMessage->version = static_cast<uint8_t>(pEntry->version);
			if(phyMessage->Clause45Enable)
				phyMessage->clause45 = pEntry->clause45;
			else
				phyMessage->clause22 = pEntry->clause22;
			msg->messages.push_back(phyMessage);
		}
	}
	return msg;
}

bool HardwareEthernetPhyRegisterPacket::EncodeFromMessage(const EthPhyMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
{
	const size_t messageCount = message.getMessageCount();
	if(!messageCount)
	{
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}
	else if (messageCount > MaxPhyEntries)
	{
		report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
		return false;
	}
	auto byteSize = (messageCount * sizeof(PhyRegisterPacket_t)) + sizeof(PhyRegisterHeader_t);
	bytestream.reserve(byteSize);
	bytestream.push_back(static_cast<uint8_t>(messageCount & 0xFF));
	bytestream.push_back(static_cast<uint8_t>((messageCount >> 8) & 0xFF));
	bytestream.push_back(PhyPacketVersion);
	bytestream.push_back(static_cast<uint8_t>(sizeof(PhyRegisterPacket_t)));
	for(auto& phyMessage : message.messages)
	{
		PhyRegisterPacket_t tempPacket;
		tempPacket.Enabled = phyMessage->Enabled ? 0x1u : 0x0u;
		tempPacket.WriteEnable = phyMessage->WriteEnable ? 0x1u : 0x0u;
		tempPacket.version = (phyMessage->version & 0xF);
		if(phyMessage->Clause45Enable)
		{
			if( (FiveBits < phyMessage->clause45.port) ||
				(FiveBits < phyMessage->clause45.device) )
			{
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			}
			tempPacket.Clause45Enable = 0x1u;
			tempPacket.clause45.port = phyMessage->clause45.port;
			tempPacket.clause45.device = phyMessage->clause45.device;
			tempPacket.clause45.regAddr = phyMessage->clause45.regAddr;
			tempPacket.clause45.regVal = phyMessage->clause45.regVal;
		}
		else
		{
			if( (FiveBits < phyMessage->clause22.phyAddr) ||
				(FiveBits < phyMessage->clause22.regAddr) )
			{
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			}
			tempPacket.Clause45Enable = 0x0u;
			tempPacket.clause22.phyAddr = phyMessage->clause22.phyAddr;
			tempPacket.clause22.page = phyMessage->clause22.page;
			tempPacket.clause22.regAddr = phyMessage->clause22.regAddr;
			tempPacket.clause22.regVal = phyMessage->clause22.regVal;
		}
		uint8_t* pktPtr = reinterpret_cast<uint8_t*>(&tempPacket);
		bytestream.insert(bytestream.end(), pktPtr, pktPtr + sizeof(PhyRegisterPacket_t));
	}
	return true;
}
}