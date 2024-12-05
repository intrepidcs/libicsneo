#include "icsneo/communication/encoder.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/message/main51message.h"
#include "icsneo/communication/packet/livedatapacket.h"
#include "icsneo/communication/packet/ethernetpacket.h"
#include "icsneo/communication/packet/iso9141packet.h"
#include "icsneo/communication/packet/canpacket.h"
#include "icsneo/communication/packet/ethphyregpacket.h"
#include "icsneo/communication/message/ethphymessage.h"
#include "icsneo/communication/packet/i2cpacket.h"
#include "icsneo/communication/message/i2cmessage.h"
#include "icsneo/communication/packet/a2bpacket.h"
#include "icsneo/communication/packet/linpacket.h"
#include "icsneo/communication/packet/mdiopacket.h"

using namespace icsneo;

bool Encoder::encode(const Packetizer& packetizer, std::vector<uint8_t>& result, const std::shared_ptr<Message>& message) {
	bool shortFormat = false;
	std::vector<uint8_t>* buffer = &result;
	uint16_t netid = 0;
	result.clear();

	switch(message->type) {
		case Message::Type::BusMessage: {
			auto frame = std::dynamic_pointer_cast<BusMessage>(message);

			// BusMessage uses frame->data as the buffer unless directed otherwise
			buffer = &frame->data;
			netid = uint16_t(frame->network.getNetID());

			switch(frame->network.getType()) {
				case icsneo_msg_bus_type_ethernet: {
					auto ethmsg = std::dynamic_pointer_cast<EthernetMessage>(message);
					if(!ethmsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false; // The message was not a properly formed EthernetMessage
					}

					buffer = &result;
					if(!HardwareEthernetPacket::EncodeFromMessage(*ethmsg, result, report))
						return false;

					break;
				} // End of icsneo_msg_bus_type_ethernet
				case icsneo_msg_bus_type_can:
				case icsneo_msg_bus_type_swcan:
				case icsneo_msg_bus_type_lsftcan: {
					auto canmsg = std::dynamic_pointer_cast<CANMessage>(message);
					if(!canmsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false; // The message was not a properly formed CANMessage
					}

					if(!supportCANFD && canmsg->isCANFD) {
						report(APIEvent::Type::CANFDNotSupported, APIEvent::Severity::Error);
						return false; // This device does not support CAN FD
					}

					buffer = &result;
					if(!HardwareCANPacket::EncodeFromMessage(*canmsg, result, report))
						return false; // The CANMessage was malformed

					break;
				} // End of icsneo_msg_bus_type_can
				case icsneo_msg_bus_type_iso9141: {
					auto isomsg = std::dynamic_pointer_cast<ISO9141Message>(message);
					if(!isomsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false; // The message was not a properly formed ISO9141Message
					}

					// Skip the normal message wrapping at the bottom since we need to send multiple
					// packets to the device. This function just encodes them back to back into `result`
					return HardwareISO9141Packet::EncodeFromMessage(*isomsg, result, report, packetizer);
				} // End of icsneo_msg_bus_type_iso9141
				case icsneo_msg_bus_type_a2b: {
					auto a2bmsg = std::dynamic_pointer_cast<A2BMessage>(message);
					if(!a2bmsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false;
					}
					buffer = &result;
					if(!HardwareA2BPacket::EncodeFromMessage(*a2bmsg, result, report)) {
						return false;
					}
					break;
				} // End of icsneo_msg_bus_type_a2b
				case icsneo_msg_bus_type_i2c: {
					auto i2cmsg = std::dynamic_pointer_cast<I2CMessage>(message);
					if(!i2cmsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false;
					}
					buffer = &result;
					if(!HardwareI2CPacket::EncodeFromMessage(*i2cmsg, result, report)) {
						return false;
					}
					break;
				} // End of icsneo_msg_bus_type_i2c
				case icsneo_msg_bus_type_lin: {
					auto linmsg = std::dynamic_pointer_cast<LINMessage>(message);
					if(!linmsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false;
					}
					buffer = &result;
					if(!HardwareLINPacket::EncodeFromMessage(*linmsg, result, report)) {
						return false;
					}
					break;
				} // End of icsneo_msg_bus_type_lin
				case icsneo_msg_bus_type_mdio: {
					auto mdiomsg = std::dynamic_pointer_cast<MDIOMessage>(message);
					if(!mdiomsg) {
						report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
						return false;
					}
					buffer = &result;
					if(!HardwareMDIOPacket::EncodeFromMessage(*mdiomsg, result, report)) {
						return false;
					}
					break;
				} // End of icsneo_msg_bus_type_mdio
				default:
					report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
					return false;
			}

			break;
		}
		case Message::Type::InternalMessage: {
			auto raw = std::dynamic_pointer_cast<InternalMessage>(message);

			// Raw message uses raw->data as the buffer unless directed otherwise
			buffer = &raw->data;
			netid = uint16_t(raw->network.getNetID());

			switch(raw->network.getNetID()) {
				case Network::_icsneo_netid_t::icsneo_netid_device:
					shortFormat = true;
					break;
				case Network::_icsneo_netid_t::icsneo_netid_red_oldformat: {
					// See the decoder for an explanation
					// We expect the network byte to be populated already in data, but not the length
					uint16_t length = uint16_t(raw->data.size()) - 1;
					raw->data.insert(raw->data.begin(), {(uint8_t)length, (uint8_t)(length >> 8)});
					break;
				}
				default:
					report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
					return false;
			}
			break;
		}
		case Message::Type::Main51: {
			auto m51msg = std::dynamic_pointer_cast<Main51Message>(message);
			if(!m51msg) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false; // The message was not a properly formed Main51Message
			}

			buffer = &m51msg->data;
			netid = uint16_t(Network::_icsneo_netid_t::icsneo_netid_main51);

			if(!m51msg->forceShortFormat) {
				// Main51 can be sent as a long message without setting the NetID to RED first
				// Size in long format is the size of the entire packet
				// So +1 for AA header, +1 for short format header, and +2 for long format size
				uint16_t size = uint16_t(m51msg->data.size()) + 1 + 1 + 2;
				size += 1; // Even though we are not including the NetID bytes, the device expects them to be counted in the length
				size += 1; // Main51 Command
				m51msg->data.insert(m51msg->data.begin(), {
					(uint8_t)Network::_icsneo_netid_t::icsneo_netid_main51, // 0x0B for long message
					(uint8_t)size, // Size, little endian 16-bit
					(uint8_t)(size >> 8),
					(uint8_t)m51msg->command
				});
				result = packetizer.packetWrap(m51msg->data, shortFormat);
				return true;
			} else {
				m51msg->data.insert(m51msg->data.begin(), { uint8_t(m51msg->command) });
				shortFormat = true;
			}
			break;
		}
		case Message::Type::EthernetPhyRegister: {
			if(!supportEthPhy) {
				report(APIEvent::Type::EthPhyRegisterControlNotAvailable, APIEvent::Severity::Error);
				return false;
			}
			auto ethPhyMessage = std::dynamic_pointer_cast<EthPhyMessage>(message);
			if(!ethPhyMessage) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false;
			}
			if(!HardwareEthernetPhyRegisterPacket::EncodeFromMessage(*ethPhyMessage, result, report))
				return false;
			break;
		}
		case Message::Type::LiveData: {
			auto liveDataMsg = std::dynamic_pointer_cast<LiveDataMessage>(message);
			if(!liveDataMsg) {
				report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
				return false; // The message was not a properly formed LiveDataMessage
			}
			if(!HardwareLiveDataPacket::EncodeFromMessage(*liveDataMsg, result, report))
				return false;
			result = packetizer.packetWrap(result, false);
			return true;
		}
		break;
	}

	// Early returns may mean we don't reach this far, check the type you're concerned with
	if(shortFormat) {
		buffer->insert(buffer->begin(), (uint8_t(buffer->size()) << 4) | uint8_t(netid));
	} else {
		// Size for the host-to-device long format is the size of the entire packet + 1
		// So +1 for AA header, +1 for short format header, +2 for long format size, and +2 for long format NetID
		// Then an extra +1, due to a firmware idiosyncrasy
		uint16_t size = static_cast<uint16_t>(buffer->size()) + 1 + 1 + 2 + 2 + 1;

		buffer->insert(buffer->begin(), {
			(uint8_t)Network::_icsneo_netid_t::icsneo_netid_red, // 0x0C for long message
			(uint8_t)size, // Size, little endian 16-bit
			(uint8_t)(size >> 8),
			(uint8_t)netid, // NetID, little endian 16-bit
			(uint8_t)(netid >> 8)
		});
	}

	result = packetizer.packetWrap(*buffer, shortFormat);
	return true;
}

bool Encoder::encode(const Packetizer& packetizer, std::vector<uint8_t>& result, Command cmd, std::vector<uint8_t> arguments) {
	std::shared_ptr<Message> msg;
	if(cmd == Command::UpdateLEDState) {
		/* _icsneo_netid_t::Device is a super old command type.
		 * It has a leading 0x00 byte, a byte for command, and a byte for an argument.
		 * In this case, command 0x06 is SetLEDState.
		 * This old command type is not really used anywhere else.
		 */
		auto canmsg = std::make_shared<InternalMessage>(Network::_icsneo_netid_t::icsneo_netid_device);
		msg = canmsg;
		if(arguments.empty()) {
			report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
			return false;
		}
		canmsg->data.reserve(3);
		canmsg->data.push_back(0x00);
		canmsg->data.push_back(0x06); // SetLEDState
		canmsg->data.push_back(arguments.at(0)); // See Device::LEDState
	} else {
		auto m51msg = std::make_shared<Main51Message>();
		msg = m51msg;
		m51msg->command = cmd;
		switch(cmd) {
			case Command::ReadSettings:
			case Command::RequestSerialNumber:
			case Command::EnableNetworkCommunication:
			case Command::EnableNetworkCommunicationEx:
			case Command::GetMainVersion:
			case Command::GetSecondaryVersions:
			case Command::NeoReadMemory:
				// There is a firmware handling idiosyncrasy with these commands
				// They must be encoded in the short format
				m51msg->forceShortFormat = true;
			default:
				break;
		}
		m51msg->data.insert(m51msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	}

	return encode(packetizer, result, msg);
}