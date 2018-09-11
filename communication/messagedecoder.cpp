#include "communication/include/messagedecoder.h"
#include "communication/include/communication.h"
#include <iostream>

using namespace icsneo;

CANMessage MessageDecoder::CoreMiniMsg::toCANMessage(Network network) {
	CANMessage msg;
	msg.network = network;
	msg.arbid = CxTRB0SID.SID;
	msg.data.reserve(CxTRB0DLC.DLC);
	for(auto i = 0; i < CxTRB0DLC.DLC; i++)
		msg.data.push_back(CxTRB0Dall[i]);
	return msg;
}

bool MessageDecoder::input(const std::vector<uint8_t>& inputBytes) {
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
					//std::cout << (int)bytes[0] << " ";
					bytes.pop_front(); // Discard
				}
				break;
			case ReadState::ParseHeader:
				if(bytes.size() < 2) {
					haveEnoughData = false;
					break;
				}

				messageLength = bytes[1] >> 4 & 0xf; // Upper nibble of the second byte denotes the message length
				message.network = Network(bytes[1] & 0xf); // Lower nibble of the second byte is the network ID
				if(messageLength == 0) { // A length of zero denotes a long style packet
					state = ReadState::ParseLongStylePacketHeader;
					checksum = false;
					headerSize = 6;
				} else {
					state = ReadState::GetData;
					checksum = true;
					headerSize = 2;
					messageLength += 2; // The message length given in short messages does not include header
				}
				currentIndex++;
				break;
			case ReadState::ParseLongStylePacketHeader:
				if(bytes.size() < 6) {
					haveEnoughData = false;
					break;
				}

				messageLength = bytes[2]; // Long messages have a little endian length on bytes 3 and 4
				messageLength |= bytes[3] << 8;
				message.network = Network((bytes[5] << 8) | bytes[4]); // Long messages have their netid stored as little endian on bytes 5 and 6
				currentIndex += 4;

				/* Long messages can't have a length less than 4, because that would indicate a negative payload size.
				 * Unlike the short message length, the long message length encompasses everything from the 0xAA to the
				 * end of the payload. The short message length, for reference, only encompasses the length of the actual
				 * payload, and not the header or checksum.
				 */
				if(messageLength < 4 || messageLength > 4000) {
					bytes.pop_front();
					//std::cout << "skipping long message with length " << messageLength << std::endl;
					state = ReadState::SearchForHeader;
				} else {
					state = ReadState::GetData;
				}
				break;
			case ReadState::GetData:
				// We do not include the checksum in messageLength so it doesn't get copied into the payload buffer
				if(bytes.size() < messageLength + (checksum ? 1 : 0)) { // Read until we have the rest of the message
					haveEnoughData = false;
					break;
				}

				message.data.clear();
				if(messageLength > 0)
					message.data.reserve(messageLength - headerSize);
				
				while(currentIndex < messageLength)
					message.data.push_back(bytes[currentIndex++]);

				if(!checksum || bytes[currentIndex] == Communication::ICSChecksum(message.data)) {
					// Got a good packet
					gotGoodMessages = true;
					processMessage(message);
					for (auto i = 0; i < messageLength; i++)
						bytes.pop_front();
					
				} else {
					if(gotGoodMessages) // Don't complain unless we've already gotten a good message, in case we started in the middle of a stream
						std::cout << "Dropping message due to bad checksum" << std::endl;
					bytes.pop_front(); // Drop the first byte so it doesn't get picked up again
				}
				
				// Reset for the next packet
				currentIndex = 0;
				state = ReadState::SearchForHeader;
				break;
		}
	}

	return processedMessages.size() > 0;
}

std::vector<std::shared_ptr<Message>> MessageDecoder::output() {
	auto ret = std::move(processedMessages);
	processedMessages = std::vector<std::shared_ptr<Message>>(); // Reset the vector
	return ret;
}

void MessageDecoder::processMessage(const Message& msg) {
	switch(msg.network.getType()) {
		case Network::Type::CAN:
			if(msg.data.size() >= 24) {
				CoreMiniMsg* cmsg = (CoreMiniMsg*)msg.data.data();
				processedMessages.push_back(std::make_shared<CANMessage>(cmsg->toCANMessage(msg.network)));
			} else {
				//std::cout << "bad CAN frame " << msg.data.size() << std::endl;
			}
			break;
		default:
			// if(msg.network.getNetID() != Network::NetID::Device)
			// 	std::cout << "Message: " << msg.network << " with data length " << msg.data.size() << std::endl;
			processedMessages.push_back(std::make_shared<Message>(msg));
	}
}