#ifndef __MDIOMESSAGE_H_
#define __MDIOMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class MDIOMessage : public BusMessage {
public:
	const BusMessage::Type getBusType() const final { return BusMessage::Type::MDIO; }
	
	enum class Clause : uint8_t {
		Clause45 = 0,
		Clause22 = 1
	};

	enum class Direction : uint8_t {
		Write = 0,
		Read = 1
	};

	bool isTXMsg = false;
	bool txTimeout = false;
	bool txAborted = false;
	bool txInvalidBus = false;
	bool txInvalidPhyAddr = false;
	bool txInvalidRegAddr = false;
	bool txInvalidClause = false;
	bool txInvalidOpcode = false;
	uint8_t phyAddress;
	uint8_t devAddress;
	uint16_t regAddress;
	Direction direction;
	Clause clause;
};

}

#endif // __cplusplus

#endif