#ifndef __SPIMESSAGE_H_
#define __SPIMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <vector>

namespace icsneo {

class SPIMessage : public Frame {
public:
	enum class Direction : uint8_t {
		Read = 0,
		Write = 1
	};

	Direction direction = Direction::Read;
	uint16_t address = static_cast<uint16_t>(0x0000u);
	uint8_t mms = static_cast<uint16_t>(0x0000u); // Memory Map Selector for ADI MAC Phy
	uint16_t stats = static_cast<uint16_t>(0x0000u);
	
	std::vector<uint32_t> payload;
};

}

#endif // __cplusplus

#endif
