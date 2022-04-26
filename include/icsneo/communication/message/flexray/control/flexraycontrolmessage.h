#ifndef __FLEXRAYCONTROLMESSAGE_H_
#define __FLEXRAYCONTROLMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/packet.h"
#include "icsneo/device/extensions/flexray/opcode.h"
#include "icsneo/device/extensions/flexray/pocstatus.h"
#include "icsneo/device/extensions/flexray/channel.h"

namespace icsneo {

// Internal message which gives us the state of the FlexRay Controllers
class FlexRayControlMessage : public Message {
public:
	static std::vector<uint8_t> BuildBaseControlArgs(uint8_t controller, FlexRay::Opcode op, const std::vector<uint8_t>& args);
	static std::vector<uint8_t> BuildReadCCRegsArgs(uint8_t controller, uint16_t startAddress, uint8_t numRegisters = 1);
	static std::vector<uint8_t> BuildWriteCCRegArgs(uint8_t controller, uint16_t address, uint32_t value);
	static std::vector<uint8_t> BuildAddConfiguredTxMessageArgs(
		uint8_t controller, uint16_t descriptionId, uint16_t slotId, uint8_t baseCycle, uint8_t cycleReps, FlexRay::Channel channel);
	static std::vector<uint8_t> BuildWriteMessageBufferArgs(
		uint8_t controller, uint16_t bufferId, const std::vector<uint8_t>& data, uint16_t desiredSize);

	FlexRayControlMessage(const Packet& packet);

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

#endif // __cplusplus

#endif