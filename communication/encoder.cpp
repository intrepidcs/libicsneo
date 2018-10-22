#include "icsneo/communication/encoder.h"

using namespace icsneo;

bool Encoder::encode(std::vector<uint8_t>& result, const std::shared_ptr<Message>& message) {
	bool shortFormat = false;
	bool useResultAsBuffer = false; // Otherwise it's expected that we use message->data
	result.clear();

	switch(message->network.getType()) {
		case Network::Type::CAN: {
			useResultAsBuffer = true;

			auto canmsg = std::dynamic_pointer_cast<CANMessage>(message);
			if(!canmsg)
				return false; // The message was not a properly formed CANMessage

			if(!supportCANFD && canmsg->isCANFD)
				return false; // This device does not support CAN FD

			if(canmsg->isCANFD && canmsg->isRemote)
				return false; // RTR frames can not be used with CAN FD

			const size_t dataSize = canmsg->data.size();
			if(dataSize > 64 || (dataSize > 8 && !canmsg->isCANFD))
				return false; // Too much data for the protocol

			uint8_t lengthNibble = uint8_t(canmsg->data.size());
			if(lengthNibble > 8) {
				switch(lengthNibble) {
					case 12:
						lengthNibble = 0x9;
						break;
					case 16:
						lengthNibble = 0xA;
						break;
					case 20:
						lengthNibble = 0xB;
						break;
					case 24:
						lengthNibble = 0xC;
						break;
					case 32:
						lengthNibble = 0xD;
						break;
					case 48:
						lengthNibble = 0xE;
						break;
					case 64:
						lengthNibble = 0xF;
						break;
					default:
						return false; // CAN FD frame may have had an incorrect byte count
				}
			}

			// Pre-allocate as much memory as we will possibly need for speed
			result.reserve(17 + dataSize);

			result.push_back(0 /* byte count here later */ << 4 | (uint8_t(canmsg->network.getNetID()) & 0xF));
			result.insert(result.end(), {0,0}); // Two bytes for Description ID, big endian, not used in API currently

			// Next 2-4 bytes are ArbID
			if(canmsg->isExtended) {
				if(canmsg->arbid >= 0x20000000) // Extended messages use 29-bit arb IDs
					return false;

				result.insert(result.end(), {
					(uint8_t)(canmsg->arbid >> 21),
					(uint8_t)(((((canmsg->arbid & 0x001C0000) >> 13) & 0xFF) + (((canmsg->arbid & 0x00030000) >> 16) & 0xFF)) | 8),
					(uint8_t)(canmsg->arbid >> 8),
					(uint8_t)canmsg->arbid
				});
			} else {
				if(canmsg->arbid >= 0x800) // Standard messages use 11-bit arb IDs
					return false;

				result.insert(result.end(), {
					(uint8_t)(canmsg->arbid >> 3),
					(uint8_t)((canmsg->arbid & 0x7) << 5)
				});
			}

			// Status and DLC bits
			if(canmsg->isCANFD) {
				result.push_back(0x0F); // FD Frame
				uint8_t fdStatusByte = lengthNibble;
				if(canmsg->baudrateSwitch)
					fdStatusByte |= 0x80; // BRS status bit
				result.push_back(fdStatusByte);
			} else {
				// TODO Support high voltage wakeup, bitwise-or in 0x8 here to enable
				uint8_t statusNibble = canmsg->isRemote ? 0x4 : 0x0;
				result.push_back((statusNibble << 4) | lengthNibble);
			}

			// Now finally the payload
			result.insert(result.end(), canmsg->data.begin(), canmsg->data.end());
			result.push_back(0);

			// Fill in the length byte from earlier
			result[0] |= result.size() << 4;
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
	msg->network = Network::NetID::Main51;
	msg->data.reserve(arguments.size() + 1);
	msg->data.push_back((uint8_t)cmd);
	msg->data.insert(msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	return encode(result, msg);
}