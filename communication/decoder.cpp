#include "communication/include/decoder.h"
#include "communication/include/communication.h"
#include "communication/message/include/serialnumbermessage.h"
#include "communication/include/command.h"
#include "device/include/device.h"
#include <iostream>

using namespace icsneo;

uint64_t Decoder::GetUInt64FromLEBytes(uint8_t* bytes) {
	uint64_t ret = 0;
	for(int i = 0; i < 8; i++)
		ret |= (bytes[i] << (i * 8));
	return ret;
}

std::shared_ptr<Message> Decoder::decodePacket(const std::shared_ptr<Packet>& packet) {
	switch(packet->network.getType()) {
		case Network::Type::CAN: {
			if(packet->data.size() < 24)
				break; // We would read garbage when interpereting the data

			HardwareCANPacket* data = (HardwareCANPacket*)packet->data.data();
			auto msg = std::make_shared<CANMessage>();
			msg->network = packet->network;
			msg->arbid = data->header.SID;
			msg->data.reserve(data->dlc.DLC);

			// Timestamp calculation
			msg->timestamp = data->timestamp.TS * 25; // Timestamps are in 25ns increments since 1/1/2007 GMT 00:00:00.0000

			for(auto i = 0; i < data->dlc.DLC; i++)
				msg->data.push_back(data->data[i]);
			return msg;
		}
		default:
			switch(packet->network.getNetID()) {
				case Network::NetID::Main51: {
					switch((Command)packet->data[0]) {
						case Command::RequestSerialNumber: {
							auto msg = std::make_shared<SerialNumberMessage>();
							msg->network = packet->network;
							uint64_t serial = GetUInt64FromLEBytes(packet->data.data() + 1);
							// The device sends 64-bits of serial number, but we never use more than 32-bits.
							msg->deviceSerial = Device::SerialNumToString((uint32_t)serial);
							msg->hasMacAddress = packet->data.size() >= 15;
							if(msg->hasMacAddress)
								memcpy(msg->macAddress, packet->data.data() + 9, sizeof(msg->macAddress));
							msg->hasPCBSerial = packet->data.size() >= 31;
							if(msg->hasPCBSerial)
								memcpy(msg->pcbSerial, packet->data.data() + 15, sizeof(msg->pcbSerial));
							return msg;
						}
					}
					break;
				}
				case Network::NetID::RED_OLDFORMAT: {
					/* So-called "old format" messages are a "new style, long format" wrapper around the old short messages.
					 * They consist of a 16-bit LE length first, then the 8-bit length and netid combo byte, then the payload
					 * with no checksum. The upper-nibble length of the combo byte should be ignored completely, using the
					 * length from the first two bytes in it's place. Ideally, we never actually send the oldformat messages
					 * out to the rest of the application as they can recursively get decoded to another message type here.
					 * Feed the result back into the decoder in case we do something special with the resultant netid.
					 */
					uint16_t length = packet->data[0] | (packet->data[1] << 8);
					packet->network = Network(packet->data[2] & 0xF);
					packet->data.erase(packet->data.begin(), packet->data.begin() + 3);
					if(packet->data.size() != length)
						packet->data.resize(length);
					return decodePacket(packet);
				}
			}
			break;
	}

	auto msg = std::make_shared<Message>();
	msg->network = packet->network;
	msg->data = packet->data;
	return msg;
}