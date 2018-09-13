#ifndef __PACKETIZER_H_
#define __PACKETIZER_H_

#include "communication/include/communication.h"
#include <queue>
#include <vector>
#include <memory>

namespace icsneo {

class Packetizer {
public:
	bool input(const std::vector<uint8_t>& bytes);
	std::vector<std::shared_ptr<Communication::Packet>> output();
	
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
	Communication::Packet packet;
	std::deque<uint8_t> bytes;

	std::vector<std::shared_ptr<Communication::Packet>> processedPackets;
};

}

#endif