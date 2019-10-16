#ifndef __FLEXRAYCONTROLMESSAGE_H_
#define __FLEXRAYCONTROLMESSAGE_H_

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/packet.h"
#include "icsneo/device/extensions/flexray/opcode.h"
#include "icsneo/device/extensions/flexray/pocstatus.h"

namespace icsneo {

// Internal message which gives us the state of the FlexRay Controllers
class FlexRayControlMessage : public Message {
public:
	static std::vector<uint8_t> BuildBaseControlArgs(uint8_t controller, FlexRay::Opcode op, std::initializer_list<uint8_t> args);
	static std::vector<uint8_t> BuildReadCCRegsArgs(uint8_t controller, uint16_t startAddress, uint8_t numRegisters = 1);
	static std::vector<uint8_t> BuildWriteCCRegArgs(uint8_t controller, uint16_t address, uint32_t value);

	FlexRayControlMessage(const Packet& packet);
	virtual ~FlexRayControlMessage() = default;
	bool decoded = false;
	uint8_t controller = 0xff; // Controller index, either 0 or 1
	FlexRay::Opcode opcode = FlexRay::Opcode::Unknown;
	FlexRay::POCStatus pocStatus = FlexRay::POCStatus::Unknown;
	uint16_t slotCounterA = 0;
	uint16_t slotCounterB = 0;
	uint16_t macroTick = 0;
	uint16_t cycleCount = 0;
	uint32_t rateCorrection = 0;
	uint32_t offsetCorrection = 0;
	std::vector<uint32_t> registers;
};

}

#endif