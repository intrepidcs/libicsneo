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
			phyMessage->BusIndex = static_cast<uint8_t>(pEntry->BusIndex);
			phyMessage->Version = static_cast<uint8_t>(pEntry->version);
			if(phyMessage->Clause45Enable)
				phyMessage->Clause45 = pEntry->clause45;
			else
				phyMessage->Clause22 = pEntry->clause22;
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
		tempPacket.BusIndex = (phyMessage->BusIndex & 0xF);
		tempPacket.version = (phyMessage->Version & 0xF);
		if(phyMessage->Clause45Enable)
		{
			if( (FiveBits < phyMessage->Clause45.port) ||
				(FiveBits < phyMessage->Clause45.device) )
			{
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			}
			tempPacket.Clause45Enable = 0x1u;
			tempPacket.clause45.port = phyMessage->Clause45.port;
			tempPacket.clause45.device = phyMessage->Clause45.device;
			tempPacket.clause45.regAddr = phyMessage->Clause45.regAddr;
			tempPacket.clause45.regVal = phyMessage->Clause45.regVal;
		}
		else
		{
			if( (FiveBits < phyMessage->Clause22.phyAddr) ||
				(FiveBits < phyMessage->Clause22.regAddr) )
			{
				report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
				return false;
			}
			tempPacket.Clause45Enable = 0x0u;
			tempPacket.clause22.phyAddr = phyMessage->Clause22.phyAddr;
			tempPacket.clause22.page = phyMessage->Clause22.page;
			tempPacket.clause22.regAddr = phyMessage->Clause22.regAddr;
			tempPacket.clause22.regVal = phyMessage->Clause22.regVal;
		}
		uint8_t* pktPtr = reinterpret_cast<uint8_t*>(&tempPacket);
		bytestream.insert(bytestream.end(), pktPtr, pktPtr + sizeof(PhyRegisterPacket_t));
	}
	return true;
}
}