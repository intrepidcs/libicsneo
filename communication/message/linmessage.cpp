#include "icsneo/communication/message/linmessage.h"
#include <numeric>

namespace icsneo {

void LINMessage::calcChecksum(LINMessage& message) {
	uint16_t sum = 0;
	auto limitFunc = [](uint16_t x, uint16_t y) -> uint16_t {
		if ((x + y) > 0xFFu)
			return ((x + y) - 0xFFu);
		else
			return (x + y);
	};

	message.checksum = static_cast<uint8_t>(std::accumulate(message.data.begin(), message.data.end(), sum, limitFunc));
	if(message.isEnhancedChecksum)
		message.checksum = static_cast<uint8_t>(limitFunc(message.checksum, message.protectedID));

	message.checksum ^= 0xFFu;
}

uint8_t LINMessage::calcProtectedID(uint8_t& id) {
	uint8_t protID = id;
	auto bit = [&](uint8_t pos)->uint8_t { return ((protID >> pos) & 0x1u); };
	protID |= (~(bit(1) ^ bit(3) ^ bit(4) ^ bit(5)) << 7);
	protID |= ((bit(0) ^ bit(1) ^ bit(2) ^ bit(4)) << 6);
	return protID;
}

} //namespace icsneo