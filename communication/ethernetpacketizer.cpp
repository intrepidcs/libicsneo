#include "icsneo/communication/ethernetpacketizer.h"
#include <algorithm>
#include <iterator>
#include <cstring>
#include <cassert>

using namespace icsneo;

const size_t EthernetPacketizer::MaxPacketLength = 1490; // MTU - overhead
static const uint8_t BROADCAST_MAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

EthernetPacketizer::EthernetPacket& EthernetPacketizer::newSendPacket(bool first) {
	processedDownPackets.emplace_back();
	EthernetPacket& ret = processedDownPackets.back();
	if(first) {
		ret.packetNumber = sequenceDown++;
	} else {
		ret.firstPiece = false;
		if(processedDownPackets.size() > 1)
			ret.packetNumber = (processedDownPackets.rbegin() + 1)->packetNumber;
		else
			assert(false); // This should never be called with !first if there are no packets in the queue
	}
	std::copy(std::begin(hostMAC), std::end(hostMAC), std::begin(ret.srcMAC));
	std::copy(std::begin(deviceMAC), std::end(deviceMAC), std::begin(ret.destMAC));
	return ret;
}

void EthernetPacketizer::inputDown(std::vector<uint8_t> bytes, bool first) {
	EthernetPacket* sendPacket = nullptr;
	if(first && !processedDownPackets.empty()) {
		// We have some packets already, let's see if we can add this to the last one
		if(processedDownPackets.back().payload.size() + bytes.size() <= MaxPacketLength)
			sendPacket = &processedDownPackets.back();
	}

	if(sendPacket == nullptr)
		sendPacket = &newSendPacket(first);

	if(sendPacket->payload.empty())
		sendPacket->payload = std::move(bytes);
	else
		sendPacket->payload.insert(sendPacket->payload.end(), bytes.begin(), bytes.end());

	// Split packets larger than MTU
	std::vector<uint8_t> extraData;
	if(sendPacket->payload.size() > MaxPacketLength) {
		extraData.insert(extraData.end(), sendPacket->payload.begin() + MaxPacketLength, sendPacket->payload.end());
		sendPacket->payload.resize(MaxPacketLength);
		sendPacket->lastPiece = false;
		inputDown(std::move(extraData), false);
	}
}

std::vector< std::vector<uint8_t> > EthernetPacketizer::outputDown() {
	std::vector< std::vector<uint8_t> > ret;
	ret.reserve(processedDownPackets.size());
	for(auto&& packet : std::move(processedDownPackets))
		ret.push_back(packet.getBytestream());
	processedDownPackets.clear();
	return ret;
}

bool EthernetPacketizer::inputUp(std::vector<uint8_t> bytes) {
	EthernetPacket packet(bytes);

	if(packet.errorWhileDecodingFromBytestream)
		return false; // Bad packet

	if(packet.etherType != 0xCAB2)
		return false; // Not a packet to host

	if(memcmp(packet.destMAC, hostMAC, sizeof(packet.destMAC)) != 0 &&
		memcmp(packet.destMAC, BROADCAST_MAC, sizeof(packet.destMAC)) != 0)
		return false; // Packet is not addressed to us or broadcast

	if(!allowInPacketsFromAnyMAC && memcmp(packet.srcMAC, deviceMAC, sizeof(deviceMAC)) != 0)
		return false; // Not a packet from the device we're concerned with

	// Handle single packets
	if(packet.firstPiece && packet.lastPiece) {
		// Could ensure no out-of-order reassembly by checking reassembing here,
		// not doing that here because it should be harmless if it ever happened.
		processedUpBytes.insert(processedUpBytes.end(), std::make_move_iterator(packet.payload.begin()), std::make_move_iterator(packet.payload.end()));
		return true;
	}

	if(packet.firstPiece) {
		if(reassembling) {
			//report(APIEvent::Type::FailedToRead, APIEvent::Severity::EventWarning);
			reassemblingData.clear();
		}

		reassembling = true;
		reassemblingId = packet.packetNumber;
		reassemblingData = std::move(packet.payload);
		return !processedUpBytes.empty(); // If there are other packets in the pipe
	}

	if(!reassembling || reassemblingId != packet.packetNumber) {
		//report(APIEvent::Type::FailedToRead, APIEvent::Severity::EventWarning);
		reassembling = false;
		reassemblingData.clear();
		return !processedUpBytes.empty(); // If there are other packets in the pipe
	}

	if(packet.lastPiece) {
		processedUpBytes.insert(processedUpBytes.end(), std::make_move_iterator(reassemblingData.begin()), std::make_move_iterator(reassemblingData.end()));
		reassemblingData.clear();
		reassembling = false;
		processedUpBytes.insert(processedUpBytes.end(), std::make_move_iterator(packet.payload.begin()), std::make_move_iterator(packet.payload.end()));
		return true;
	}

	reassemblingData.insert(reassemblingData.end(), std::make_move_iterator(packet.payload.begin()), std::make_move_iterator(packet.payload.end()));
	return !processedUpBytes.empty(); // If there are other packets in the pipe
}

std::vector<uint8_t> EthernetPacketizer::outputUp() {
	std::vector<uint8_t> ret = std::move(processedUpBytes);
	processedUpBytes.clear();
	return ret;
}

EthernetPacketizer::EthernetPacket::EthernetPacket(const std::vector<uint8_t>& bytestream) {
	loadBytestream(bytestream);
}

EthernetPacketizer::EthernetPacket::EthernetPacket(const uint8_t* data, size_t size) {
	std::vector<uint8_t> bs(data, data + size);
	loadBytestream(bs);
}

int EthernetPacketizer::EthernetPacket::loadBytestream(const std::vector<uint8_t>& bytestream) {
	errorWhileDecodingFromBytestream = 0;
	if (bytestream.size() < 24) {
		errorWhileDecodingFromBytestream = 1;
		return errorWhileDecodingFromBytestream;
	}
	for(size_t i = 0; i < 6; i++)
		destMAC[i] = bytestream[i];
	for(size_t i = 0; i < 6; i++)
		srcMAC[i] = bytestream[i + 6];
	etherType = (bytestream[12] << 8) | bytestream[13];
	icsEthernetHeader = (bytestream[14] << 24) | (bytestream[15] << 16) | (bytestream[16] << 8) | bytestream[17];
	payloadSize = bytestream[18] | (bytestream[19] << 8);
	packetNumber = bytestream[20] | (bytestream[21] << 8);
	uint16_t packetInfo = bytestream[22] | (bytestream[23] << 8);
	firstPiece = packetInfo & 1;
	lastPiece = (packetInfo >> 1) & 1;
	bufferHalfFull = (packetInfo >> 2) & 2;
	payload = std::vector<uint8_t>(bytestream.begin() + 24, bytestream.end());
	size_t payloadActualSize = payload.size();
	if(payloadActualSize > payloadSize)
		payload.resize(payloadSize);
	return errorWhileDecodingFromBytestream;
}

std::vector<uint8_t> EthernetPacketizer::EthernetPacket::getBytestream() const {
	uint16_t actualPayloadSize = uint16_t(payload.size());
	std::vector<uint8_t> bytestream;
	bytestream.reserve(6 + 6 + 2 + 4 + 2 + 2 + 2 + actualPayloadSize);
	for(size_t i = 0; i < 6; i++)
		bytestream.push_back(destMAC[i]);
	for(size_t i = 0; i < 6; i++)
		bytestream.push_back(srcMAC[i]);
	// EtherType should be put into the bytestream as big endian
	bytestream.push_back((uint8_t)(etherType >> 8));
	bytestream.push_back((uint8_t)(etherType));
	// Our Ethernet header should be put into the bytestream as big endian
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 24));
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 16));
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 8));
	bytestream.push_back((uint8_t)(icsEthernetHeader));
	uint16_t declaredPayloadSize = payloadSize ? payloadSize : actualPayloadSize;
	// The payload size comes next, it's little endian
	bytestream.push_back((uint8_t)(declaredPayloadSize));
	bytestream.push_back((uint8_t)(declaredPayloadSize >> 8));
	// Packet number is little endian
	bytestream.push_back((uint8_t)(packetNumber));
	bytestream.push_back((uint8_t)(packetNumber >> 8));
	// Packet info gets assembled into a bitfield
	uint16_t packetInfo = 0;
	packetInfo |= firstPiece & 1;
	packetInfo |= (lastPiece & 1) << 1;
	packetInfo |= (bufferHalfFull & 1) << 2;
	packetInfo |= 1 << 8; // Protocol version 1
	bytestream.push_back((uint8_t)(packetInfo));
	bytestream.push_back((uint8_t)(packetInfo >> 8));
	bytestream.insert(bytestream.end(), payload.begin(), payload.end());
	return bytestream;
}