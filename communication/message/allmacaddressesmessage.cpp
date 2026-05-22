#include "icsneo/communication/message/allmacaddressesmessage.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/network.h"

#include <cstring>

using namespace icsneo;

#pragma pack(push, 2)
struct ResponseHeader {
	ExtendedCommand command;
	uint16_t length;
	uint16_t count;
};

struct MacAddrEntryPacket {
	uint16_t networkId;
	uint8_t address[MACAddressLength];
};
#pragma pack(pop)

std::shared_ptr<AllMACAddressesMessage> AllMACAddressesMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(ResponseHeader))
		return nullptr;

	const auto* hdr = reinterpret_cast<const ResponseHeader*>(bytestream.data());

	if(hdr->command != ExtendedCommand::GetAllMACAddresses)
		return nullptr;

	if(hdr->count > MaxMACAddressCount)
		return nullptr;

	const size_t required = sizeof(ResponseHeader) + hdr->count * sizeof(MacAddrEntryPacket);
	if(bytestream.size() < required)
		return nullptr;

	auto msg = std::make_shared<AllMACAddressesMessage>();

	const auto* entries = reinterpret_cast<const MacAddrEntryPacket*>(bytestream.data() + sizeof(ResponseHeader));
	for(uint16_t i = 0; i < hdr->count; ++i) {
		// The firmware sends CoreMini network IDs — convert to NetID for consistent use in libicsneo
		Network::NetID netId = Network::GetNetIDFromCoreMiniNetwork(static_cast<Network::CoreMini>(entries[i].networkId));
		auto addr = entries[i].address;
		MACAddress arrAddr; 
		std::copy(addr, addr + MACAddressLength, arrAddr.begin());
		msg->addresses.emplace(std::make_pair(netId, arrAddr));
	}

	return msg;
}
