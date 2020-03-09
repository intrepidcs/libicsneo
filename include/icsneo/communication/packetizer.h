#ifndef __PACKETIZER_H_
#define __PACKETIZER_H_

#include "icsneo/communication/packet.h"
#include "icsneo/api/eventmanager.h"
#include <queue>
#include <vector>
#include <memory>

namespace icsneo {

class Packetizer {
public:
	static uint8_t ICSChecksum(const std::vector<uint8_t>& data);

	Packetizer(device_eventhandler_t report) : report(report) {}

	std::vector<uint8_t>& packetWrap(std::vector<uint8_t>& data, bool shortFormat) const;

	bool input(const std::vector<uint8_t>& bytes);
	std::vector<std::shared_ptr<Packet>> output();

	bool disableChecksum = false; // Even for short packets
	bool align16bit = true; // Not needed for Gigalog, Galaxy, etc and newer
	
private:
	enum class ReadState {
		SearchForHeader,
		ParseHeader,
		ParseLongStylePacketHeader,
		GetData
	};
	ReadState state = ReadState::SearchForHeader;

	int currentIndex = 0;
	int packetLength = 0;
	int headerSize = 0;
	bool checksum = false;
	bool gotGoodPackets = false; // Tracks whether we've ever gotten a good packet
	Packet packet;
	std::deque<uint8_t> bytes;

	std::vector<std::shared_ptr<Packet>> processedPackets;

	device_eventhandler_t report;
};

}

#endif