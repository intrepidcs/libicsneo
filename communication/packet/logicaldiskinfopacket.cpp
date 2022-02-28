#include "icsneo/communication/packet/logicaldiskinfopacket.h"

using namespace icsneo;

std::shared_ptr<LogicalDiskInfoMessage> LogicalDiskInfoPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	// Make sure we have enough to read the packet length first
	if(bytestream.size() < sizeof(LogicalDiskInfoPacket))
		return {};

	const LogicalDiskInfoPacket* packet = reinterpret_cast<const LogicalDiskInfoPacket*>(bytestream.data());
	return std::make_shared<LogicalDiskInfoMessage>(packet->isConnected != 0, packet->numSectors, packet->hiddenSectors, packet->bytesPerSector);
}