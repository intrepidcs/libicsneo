#include "icsneo/communication/packetizer.h"
#include <iostream>
#include <iomanip>

using namespace icsneo;

uint8_t Packetizer::ICSChecksum(const std::vector<uint8_t>& data) {
	uint32_t checksum = 0;
	for(size_t i = 0; i < data.size(); i++)
		checksum += data[i];
	checksum = ~checksum;
	checksum++;
	return (uint8_t)checksum;
}

std::vector<uint8_t>& Packetizer::packetWrap(std::vector<uint8_t>& data, bool shortFormat) {
	if(shortFormat) {
		// Some devices don't use the checksum, so might as well not calculate it if that's the case
		// Either way the byte is still expected to be present in the bytestream for short messages
		data.push_back(disableChecksum ? 0x00 : ICSChecksum(data)); 
	}
	data.insert(data.begin(), 0xAA);
	if(align16bit && data.size() % 2 == 1) // Some devices always expect 16-bit aligned data
		data.push_back('A'); // Used as padding character, ignored by the firmware
	return data;
}

bool Packetizer::input(const std::vector<uint8_t>& inputBytes) {
	bool haveEnoughData = true;
	bytes.insert(bytes.end(), inputBytes.begin(), inputBytes.end());

	while(haveEnoughData) {
		switch(state) {
			case ReadState::SearchForHeader:
				if(bytes.size() < 1) {
					haveEnoughData = false;
					break;
				}

				if(bytes[0] == 0xAA) { // 0xAA denotes the beginning of a packet
					state = ReadState::ParseHeader;
					currentIndex = 1;
				} else {
					bytes.pop_front(); // Discard
				}
				break;
			case ReadState::ParseHeader:
				if(bytes.size() < 2) {
					haveEnoughData = false;
					break;
				}

				packetLength = bytes[1] >> 4 & 0xf; // Upper nibble of the second byte denotes the packet length
				packet.network = Network(bytes[1] & 0xf); // Lower nibble of the second byte is the network ID
				if(packetLength == 0) { // A length of zero denotes a long style packet
					state = ReadState::ParseLongStylePacketHeader;
					checksum = false;
					headerSize = 6;
				} else {
					state = ReadState::GetData;
					checksum = true; // Even if checksum is not explicitly disallowed, we enable it here, as this goes into length calculation
					headerSize = 2;
					packetLength += 2; // The packet length given in short packets does not include header
				}
				currentIndex++;
				break;
			case ReadState::ParseLongStylePacketHeader:
				if(bytes.size() < 6) {
					haveEnoughData = false;
					break;
				}

				packetLength = bytes[2]; // Long packets have a little endian length on bytes 3 and 4
				packetLength |= bytes[3] << 8;
				packet.network = Network((bytes[5] << 8) | bytes[4]); // Long packets have their netid stored as little endian on bytes 5 and 6
				currentIndex += 4;

				/* Long packets can't have a length less than 4, because that would indicate a negative payload size.
				 * Unlike the short packet length, the long packet length encompasses everything from the 0xAA to the
				 * end of the payload. The short packet length, for reference, only encompasses the length of the actual
				 * payload, and not the header or checksum.
				 */
				if(packetLength < 4 || packetLength > 4000) {
					bytes.pop_front();
					//std::cout << "skipping long packet with length " << packetLength << std::endl;
					state = ReadState::SearchForHeader;
				} else {
					state = ReadState::GetData;
				}
				break;
			case ReadState::GetData:
				// We do not include the checksum in packetLength so it doesn't get copied into the payload buffer
				if(bytes.size() < (size_t)(packetLength + (checksum ? 1 : 0))) { // Read until we have the rest of the packet
					haveEnoughData = false;
					break;
				}

				packet.data.clear();
				if(packetLength > 0)
					packet.data.resize(packetLength - headerSize);
				
				auto i = 0;
				while(currentIndex < packetLength)
					packet.data[i++] = bytes[currentIndex++];

				if(disableChecksum || !checksum || bytes[currentIndex] == ICSChecksum(packet.data)) {
					// Got a good packet
					gotGoodPackets = true;
					processedPackets.push_back(std::make_shared<Packet>(packet));
					for (auto a = 0; a < packetLength; a++)
						bytes.pop_front();
				} else {
					if(gotGoodPackets) // Don't complain unless we've already gotten a good packet, in case we started in the middle of a stream
						report(APIEvent::Type::PacketChecksumError, APIEvent::Severity::Error);
					bytes.pop_front(); // Drop the first byte so it doesn't get picked up again
				}
				
				// Reset for the next packet
				currentIndex = 0;
				state = ReadState::SearchForHeader;
				break;
		}
	}

	return processedPackets.size() > 0;
}

std::vector<std::shared_ptr<Packet>> Packetizer::output() {
	auto ret = std::move(processedPackets);
	processedPackets = std::vector<std::shared_ptr<Packet>>(); // Reset the vector
	return ret;
}