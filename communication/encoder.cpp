#include "communication/include/encoder.h"

using namespace icsneo;

std::shared_ptr<Message> Encoder::encode(const std::shared_ptr<Message>& message) {
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
					if(message->data.size() > 0xF) {
						message->network = Network::NetID::RED_OLDFORMAT;
						message->data.insert(message->data.begin(), 0x10 | (uint8_t)Network::NetID::Main51);
						return encode(message);
					}
					message->data.insert(message->data.begin(), (message->data.size() << 4) | (uint8_t)Network::NetID::Main51);
					data.in
					break;
				case Network::NetID::RED_OLDFORMAT: {
					// See the decoder for an explanation
					uint16_t length = message->data[0] | (message->data[1] << 8);
					message->network = Network(message->data[2] & 0xF);
					message->data.erase(message->data.begin(), message->data.begin() + 3);
					if(message->data.size() != length)
						message->data.resize(length);
					return decodePacket(message);
				}
			}
			break;
	}

	auto msg = std::make_shared<Message>();
	msg->network = message->network;
	msg->data = message->data;
	return msg;
}

std::vector<uint8_t> Encoder::encode(Command cmd, std::vector<uint8_t> arguments = {}) {
	auto msg = std::make_shared<Message>();
	msg->network = Network::NetID::Main51;
	msg->data.resize(arguments.size() + 1);
	msg->data.push_back((uint8_t)cmd);
	msg->data.insert(msg->data.end(), std::make_move_iterator(arguments.begin()), std::make_move_iterator(arguments.end()));
	return encode(msg);
}