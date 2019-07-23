#include "icsneo/communication/encoder.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/packet/ethernetpacket.h"
#include "icsneo/communication/packet/canpacket.h"

using namespace icsneo;

bool Encoder::encode(std::vector<uint8_t>& result, const std::shared_ptr<Message>& message) {
	bool shortFormat = false;
	bool useResultAsBuffer = false; // Otherwise it's expected that we use message->data
	result.clear();

	switch(message->network.getType()) {
		case Network::Type::Ethernet: {
			auto ethmsg = std::dynamic_pointer_cast<EthernetMessage>(message);
			if(!ethmsg) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false; // The message was not a properly formed EthernetMessage
			}

			useResultAsBuffer = true;
			if(!HardwareEthernetPacket::EncodeFromMessage(*ethmsg, result, report))
				return false;

			break;
		} // End of Network::Type::Ethernet
		case Network::Type::CAN:
		case Network::Type::SWCAN:
		case Network::Type::LSFTCAN: {
			auto canmsg = std::dynamic_pointer_cast<CANMessage>(message);
			if(!canmsg) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false; // The message was not a properly formed CANMessage
			}

			if(!supportCANFD && canmsg->isCANFD) {
				report(APIEvent::Type::CANFDNotSupported, APIEvent::Severity::Error);
				return false; // This device does not support CAN FD
			}
			
			useResultAsBuffer = true;
			if(!HardwareCANPacket::EncodeFromMessage(*canmsg, result, report))
				return false; // The CANMessage was malformed

			break;
		} // End of Network::Type::CAN
		default:
			switch(message->network.getNetID()) {
				case Network::NetID::Device:
					shortFormat = true;
					break;
				case Network::NetID::Main51:
					if(message->data.size() > 0xF) {
						// Main51 can be sent as a long message without setting the NetID to RED first
						// Size in long format is the size of the entire packet
						// So +1 for AA header, +1 for short format header, and +2 for long format size
						uint16_t size = uint16_t(message->data.size()) + 1 + 1 + 2;
						size += 1; // Even though we are not including the NetID bytes, the device expects them to be counted in the length
						message->data.insert(message->data.begin(), {
							(uint8_t)Network::NetID::Main51, // 0x0B for long message
							(uint8_t)size, // Size, little endian 16-bit
							(uint8_t)(size >> 8)
						});
						result = packetizer->packetWrap(message->data, shortFormat);
						return true;
					} else {
						shortFormat = true;
					}
					break;
				case Network::NetID::RED_OLDFORMAT: {
					// See the decoder for an explanation
					// We expect the network byte to be populated already in data, but not the length
					uint16_t length = uint16_t(message->data.size()) - 1;
					message->data.insert(message->data.begin(), {(uint8_t)length, (uint8_t)(length >> 8)});
					break;
				}
				default:
					report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
					return false;
			}
	}

	auto& buffer = useResultAsBuffer ? result : message->data;

	if(shortFormat) {
		buffer.insert(buffer.begin(), (uint8_t(buffer.size()) << 4) | uint8_t(message->network.getNetID()));
	} else {
		// Size in long format is the size of the entire packet
		// So +1 for AA header, +1 for short format header, +2 for long format size, and +2 for long format NetID
		uint16_t size = uint16_t(buffer.size()) + 1 + 1 + 2 + 2;
		buffer.insert(buffer.begin(), {
			(uint8_t)Network::NetID::RED, // 0x0C for long message
			(uint8_t)size, // Size, little endian 16-bit
			(uint8_t)(size >> 8),
			(uint8_t)message->network.getNetID(), // NetID, little endian 16-bit
			(uint8_t)(uint16_t(message->network.getNetID()) >> 8)
		});
	}

	result = packetizer->packetWrap(buffer, shortFormat);
	return true;
}

bool Encoder::encode(std::vector<uint8_t>& result, Command cmd, std::vector<uint8_t> arguments) {
	auto msg = std::make_shared<Message>();
	if(cmd == Command::UpdateLEDState) {
		/* NetID::Device is a super old command type.
		 * It has a leading 0x00 byte, a byte for command, and a byte for an argument.
		 * In this case, command 0x06 is SetLEDState.
		 * This old command type is not really used anywhere else.
		 */
		if (arguments.empty()) {
			report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
			return false;
		}
		msg->network = Network::NetID::Device;
		msg->data.reserve(3);
		msg->data.push_back(0x00);
		msg->data.push_back(0x06); // SetLEDState
		msg->data.push_back(arguments.at(0)); // See Device::LEDState
	} else {
		msg->network = Network::NetID::Main51;
		msg->data.reserve(arguments.size() + 1);
		msg->data.push_back((uint8_t)cmd);
		msg->data.insert(msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	}
	
	return encode(result, msg);
}