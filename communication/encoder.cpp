#include "communication/include/encoder.h"

using namespace icsneo;

std::vector<uint8_t> Encoder::encode(const std::shared_ptr<Message>& message) {
	bool shortFormat = false;
	switch(message->network.getType()) {
		// case Network::Type::CAN: {
		// 	if(message->data.size() < 24)
		// 		break; // We would read garbage when interpereting the data

		// 	HardwareCANPacket* data = (HardwareCANPacket*)message->data.data();
		// 	auto msg = std::make_shared<CANMessage>();
		// 	msg->network = message->network;
		// 	msg->arbid = data->header.SID;
		// 	msg->data.reserve(data->dlc.DLC);

		// 	// Timestamp calculation
		// 	msg->timestamp = data->timestamp.TS * 25; // Timestamps are in 25ns increments since 1/1/2007 GMT 00:00:00.0000

		// 	for(auto i = 0; i < data->dlc.DLC; i++)
		// 		msg->data.push_back(data->data[i]);
		// 	return msg;
		// }
		default:
			switch(message->network.getNetID()) {
				case Network::NetID::Main51:
					if(message->data.size() <= 0xF)
						shortFormat = true;
					break;
				case Network::NetID::RED_OLDFORMAT: {
					// See the decoder for an explanation
					// We expect the network byte to be populated already in data, but not the length
					uint16_t length = message->data.size() - 1;
					message->data.insert(message->data.begin(), {(uint8_t)length, (uint8_t)(length >> 8)});
				}
				default:
					break;
			}
			break;
	}

	if(shortFormat) {
		message->data.insert(message->data.begin(), (message->data.size() << 4) | (uint8_t)message->network.getNetID());
	} else {
		// Size in long format is the size of the entire packet
		// So +1 for AA header, +1 for short format header, +2 for long format size, and +2 for long format NetID
		uint16_t size = message->data.size() + 1 + 1 + 2 + 2;
		message->data.insert(message->data.begin(), {
			(uint8_t)Network::NetID::RED, // 0x0C for long message
			(uint8_t)size, // Size, little endian 16-bit
			(uint8_t)(size >> 8),
			(uint8_t)message->network.getNetID(), // NetID, little endian 16-bit
			(uint8_t)(uint16_t(message->network.getNetID()) >> 8)
		});
	}

	return packetizer->packetWrap(message->data, shortFormat);
}

std::vector<uint8_t> Encoder::encode(Command cmd, std::vector<uint8_t> arguments) {
	auto msg = std::make_shared<Message>();
	msg->network = Network::NetID::Main51;
	msg->data.resize(arguments.size() + 1);
	msg->data.push_back((uint8_t)cmd);
	msg->data.insert(msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	return encode(msg);
}