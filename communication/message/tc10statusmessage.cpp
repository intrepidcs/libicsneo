#include "icsneo/communication/message/tc10statusmessage.h"
#include "icsneo/communication/command.h"

using namespace icsneo;

#pragma pack(push, 2)
struct Header {
	ExtendedCommand command;
	uint16_t length;
};

struct Packet {
	Header header;
	TC10WakeStatus wakeStatus;
	TC10SleepStatus sleepStatus;
};
#pragma pack(pop)

std::shared_ptr<TC10StatusMessage> TC10StatusMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(Packet)) {
		return nullptr;
	}
	const Packet* packet = (Packet*)bytestream.data();
	if (packet->header.command != ExtendedCommand::GetTC10Status) {
		return nullptr;
	}
	if (packet->header.length < sizeof(Packet) - sizeof(Header)) {
		return nullptr;
	}
	return std::make_shared<TC10StatusMessage>(packet->wakeStatus, packet->sleepStatus);
}
