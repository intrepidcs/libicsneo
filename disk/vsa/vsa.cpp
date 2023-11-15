#include "icsneo/disk/vsa/vsa.h"
#include "icsneo/communication/packet/ethernetpacket.h"

using namespace icsneo;

// VSA Base Class Functions

// VSAMessage Class Functions
std::shared_ptr<Packet> VSAMessage::getPacket() const
{
	auto packet = std::make_shared<Packet>();
	packet->network = network;
	reservePacketData(packet);
	packet->data.insert(packet->data.end(), payload.begin(), payload.end());
	return packet;
}

// VSAExtendedMessage Class Functions

void VSAExtendedMessage::appendPacket(std::shared_ptr<Packet> packet) const
{
	packet->data.insert(packet->data.end(), payload.begin(), payload.end());
	// Set the network if not already set (Happens in AA0F records)
	if(packet->network.getNetID() == Network::NetID::Invalid) {
		packet->network = network;
	}
}

void VSAExtendedMessage::truncatePacket(std::shared_ptr<Packet> packet)
{
	static constexpr auto EthernetLengthOffset = 26u;
	switch(packet->network.getType()) {
		case Network::Type::Ethernet:
			const auto& packetLength = *reinterpret_cast<uint16_t*>(packet->data.data() + EthernetLengthOffset);
			const size_t ethernetFrameSize = packetLength - (sizeof(uint16_t) * 2);
			const size_t bytestreamExpectedSize = sizeof(HardwareEthernetPacket) + ethernetFrameSize;
			packet->data.resize(bytestreamExpectedSize);
	}
}