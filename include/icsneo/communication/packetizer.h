#ifndef __PACKETIZER_H_
#define __PACKETIZER_H_

#ifdef __cplusplus

#include "icsneo/communication/packet.h"
#include "icsneo/communication/ringbuffer.h"
#include "icsneo/api/eventmanager.h"
#include <queue>
#include <vector>
#include <memory>
#include <cstring>

namespace icsneo {

class Packetizer {
public:
	static uint8_t ICSChecksum(const std::vector<uint8_t>& data);

	Packetizer(device_eventhandler_t report) : report(report) {}

	std::vector<uint8_t>& packetWrap(std::vector<uint8_t>& data, bool shortFormat) const;

	bool input(RingBuffer& bytes);
	std::vector<std::shared_ptr<Packet>> output();

	bool disableChecksum = false; // Even for short packets
	bool align16bit = true; // Not needed for Mars, Galaxy, etc and newer
	
private:
	enum class ReadState {
		SearchForHeader,
		ParseHeader,
		ParseLongStylePacketHeader,
		ParseDiskDataHeader,
		GetData
	};

	ReadState state = ReadState::SearchForHeader;

	int currentIndex = 0;
	int packetLength = 0;
	int headerSize = 0;
	bool checksum = false;
	bool gotGoodPackets = false; // Tracks whether we've ever gotten a good packet
	Packet packet;

	std::vector<std::shared_ptr<Packet>> processedPackets;

	device_eventhandler_t report;
};

}

#endif // __cplusplus

#endif