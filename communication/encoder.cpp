#include "icsneo/communication/encoder.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/message/main51message.h"
#include "icsneo/communication/packet/ethernetpacket.h"
#include "icsneo/communication/packet/iso9141packet.h"
#include "icsneo/communication/packet/canpacket.h"

using namespace icsneo;

bool Encoder::encode(const Packetizer& packetizer, std::vector<uint8_t>& result, const std::shared_ptr<Message>& message) {
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
		case Network::Type::ISO9141: {
			auto isomsg = std::dynamic_pointer_cast<ISO9141Message>(message);
			if(!isomsg) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false; // The message was not a properly formed ISO9141Message
			}

			// Skip the normal message wrapping at the bottom since we need to send multiple
			// packets to the device. This function just encodes them back to back into `result`
			return HardwareISO9141Packet::EncodeFromMessage(*isomsg, result, report, packetizer);
		} // End of Network::Type::ISO9141
		default:
			switch(message->network.getNetID()) {
				case Network::NetID::Device:
					shortFormat = true;
					break;
				case Network::NetID::Main51: {
					auto m51msg = std::dynamic_pointer_cast<Main51Message>(message);
					if(!m51msg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false; // The message was not a properly formed Main51Message
					}

					if(!m51msg->forceShortFormat) {
						// Main51 can be sent as a long message without setting the NetID to RED first
						// Size in long format is the size of the entire packet
						// So +1 for AA header, +1 for short format header, and +2 for long format size
						uint16_t size = uint16_t(message->data.size()) + 1 + 1 + 2;
						size += 1; // Even though we are not including the NetID bytes, the device expects them to be counted in the length
						size += 1; // Main51 Command
						message->data.insert(message->data.begin(), {
							(uint8_t)Network::NetID::Main51, // 0x0B for long message
							(uint8_t)size, // Size, little endian 16-bit
							(uint8_t)(size >> 8),
							(uint8_t)m51msg->command
						});
						result = packetizer.packetWrap(message->data, shortFormat);
						return true;
					} else {
						message->data.insert(message->data.begin(), { uint8_t(m51msg->command) });
						shortFormat = true;
					}
					break;
				}
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

	// Early returns may mean we don't reach this far, check the type you're concerned with
	auto& buffer = useResultAsBuffer ? result : message->data;

	if(shortFormat) {
		buffer.insert(buffer.begin(), (uint8_t(buffer.size()) << 4) | uint8_t(message->network.getNetID()));
	} else {
		// Size for the host-to-device long format is the size of the entire packet + 1
		// So +1 for AA header, +1 for short format header, +2 for long format size, and +2 for long format NetID
		// Then an extra +1, due to a firmware idiosyncrasy
		uint16_t size = uint16_t(buffer.size()) + 1 + 1 + 2 + 2 + 1;
		buffer.insert(buffer.begin(), {
			(uint8_t)Network::NetID::RED, // 0x0C for long message
			(uint8_t)size, // Size, little endian 16-bit
			(uint8_t)(size >> 8),
			(uint8_t)message->network.getNetID(), // NetID, little endian 16-bit
			(uint8_t)(uint16_t(message->network.getNetID()) >> 8)
		});
	}

	result = packetizer.packetWrap(buffer, shortFormat);
	return true;
}

bool Encoder::encode(const Packetizer& packetizer, std::vector<uint8_t>& result, Command cmd, std::vector<uint8_t> arguments) {
	std::shared_ptr<Message> msg;
	if(cmd == Command::UpdateLEDState) {
		/* NetID::Device is a super old command type.
		 * It has a leading 0x00 byte, a byte for command, and a byte for an argument.
		 * In this case, command 0x06 is SetLEDState.
		 * This old command type is not really used anywhere else.
		 */
		msg = std::make_shared<Message>();
		if(arguments.empty()) {
			report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
			return false;
		}
		msg->network = Network::NetID::Device;
		msg->data.reserve(3);
		msg->data.push_back(0x00);
		msg->data.push_back(0x06); // SetLEDState
		msg->data.push_back(arguments.at(0)); // See Device::LEDState
	} else {
		auto m51msg = std::make_shared<Main51Message>();
		msg = m51msg;
		msg->network = Network::NetID::Main51;
		m51msg->command = cmd;
		switch(cmd) {
			case Command::ReadSettings:
			case Command::RequestSerialNumber:
			case Command::EnableNetworkCommunication:
			case Command::EnableNetworkCommunicationEx:
			case Command::GetMainVersion:
			case Command::GetSecondaryVersions:
				// There is a firmware handling idiosyncrasy with these commands
				// They must be encoded in the short format
				m51msg->forceShortFormat = true;
			default:
				break;
		}
		msg->data.insert(msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	}
	
	return encode(packetizer, result, msg);
}