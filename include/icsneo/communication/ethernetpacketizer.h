#ifndef __ETHERNETPACKETIZER_H_
#define __ETHERNETPACKETIZER_H_

#ifdef __cplusplus

#include "icsneo/api/eventmanager.h"
#include <queue>
#include <vector>
#include <memory>

namespace icsneo {

/**
 * A layer of encapsulation on top of the standard packetizer
 * for 0xCAB1/0xCAB2 Ethernet packets.
 * 
 * Not thread-safe by default.
 */
class EthernetPacketizer {
public:
	EthernetPacketizer(device_eventhandler_t report) : report(report) {}

	/**
	 * Call with as many packets as desired before calling
	 * outputDown to get the results. Passing in multiple
	 * packets may result in better packing.
	 */
	void inputDown(std::vector<uint8_t> bytes);
	std::vector< std::vector<uint8_t> > outputDown();

	/**
	 * Call with packet data, the packet may be queued waiting
	 * for reassembly. In this case, false will be returned.
	 */
	bool inputUp(std::vector<uint8_t> bytes);
	std::vector<uint8_t> outputUp();

	class EthernetPacket {
	public: // Don't worry about endian when setting fields, this is all taken care of in getBytestream
		EthernetPacket() {};
		EthernetPacket(const std::vector<uint8_t>& bytestream);
		EthernetPacket(const uint8_t* data, size_t size);
		int loadBytestream(const std::vector<uint8_t>& bytestream);
		std::vector<uint8_t> getBytestream() const;
		uint8_t errorWhileDecodingFromBytestream = 0; // Not part of final bytestream, only for checking the result of the constructor
		uint8_t destMAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		uint8_t srcMAC[6] = { 0x00, 0xFC, 0x70, 0xFF, 0xFF, 0xFF };
		uint16_t etherType = 0xCAB1; // Big endian, Should be 0xCAB1 or 0xCAB2
		uint32_t icsEthernetHeader = 0xAAAA5555; // Big endian, Should be 0xAAAA5555
		// At this point in the packet, there is a 16-bit payload size, little endian
		// This is calculated from payload size in getBytestream
		uint16_t packetNumber = 0;
		bool firstPiece = true; // These booleans make up a 16-bit bitfield, packetInfo
		bool lastPiece = true;
		bool bufferHalfFull = false;
		std::vector<uint8_t> payload;
	};

	uint8_t hostMAC[6] = { 0x00, 0xFC, 0x70, 0xFF, 0xFF, 0xFF };
	uint8_t deviceMAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	
private:
	bool reassembling = false;
	uint16_t reassemblingId = 0;
	std::vector<uint8_t> reassemblingData;
	uint16_t sequenceDown = 0;

	std::vector<uint8_t> processedUpBytes;
	std::vector< std::vector<uint8_t> > processedDownPackets;

	device_eventhandler_t report;
};

}

#endif // __cplusplus

#endif