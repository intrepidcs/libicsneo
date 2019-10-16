#include <icsneo/communication/message/flexray/control/flexraycontrolmessage.h>
#include <cstring> // memcpy

using namespace icsneo;

std::vector<uint8_t> FlexRayControlMessage::BuildBaseControlArgs(uint8_t controller, FlexRay::Opcode op, std::initializer_list<uint8_t> args) {
	std::vector<uint8_t> ret;
	ret.reserve(args.size() + 4);
	ret.push_back(controller);
	const uint16_t size = args.size() + 1; // Add 1 for the opcode
	ret.push_back(uint8_t(size));
	ret.push_back(uint8_t(size >> 8));
	ret.push_back(uint8_t(op));
	ret.insert(ret.end(), args.begin(), args.end());
	return ret;
}

std::vector<uint8_t> FlexRayControlMessage::BuildReadCCRegsArgs(uint8_t controller, uint16_t startAddress, uint8_t numRegisters) {
	startAddress /= 4;
	return BuildBaseControlArgs(controller, FlexRay::Opcode::ReadCCRegs, {
		uint8_t(startAddress),
		uint8_t(startAddress >> 8),
		numRegisters
	});
}

std::vector<uint8_t> FlexRayControlMessage::BuildWriteCCRegArgs(uint8_t controller, uint16_t address, uint32_t value) {
	address /= 4;
	return BuildBaseControlArgs(controller, FlexRay::Opcode::ReadCCRegs, {
		uint8_t(address),
		uint8_t(address >> 8),
		uint8_t(value),
		uint8_t(value >> 8),
		uint8_t(value >> 16),
		uint8_t(value >> 24)
	});
}

FlexRayControlMessage::FlexRayControlMessage(const Packet& packet) : Message() {
	if(packet.data.size() < 2)
		return; // huh?
	controller = packet.data[0];
	if(controller < 2)
		return; // Invalid controller

	// Opcode is only ReadCCStatus or ReadCCRegs for the moment
	opcode = FlexRay::Opcode(packet.data[1]);
	if(opcode != FlexRay::Opcode::ReadCCRegs && opcode != FlexRay::Opcode::ReadCCStatus)
		return;
	
	// Read out registers
	size_t bytes = packet.data.size() - 2;
	const size_t count = bytes / sizeof(uint32_t);
	bytes -= bytes % sizeof(uint32_t); // trim off any trailing bytes
	registers.resize(count);
	memcpy(registers.data(), packet.data.data() + 2, bytes);

	// If it was a status message, we should decode these registers into their components
	if(opcode == FlexRay::Opcode::ReadCCStatus) {
		if(count < 8)
			return;
		pocStatus = FlexRay::POCStatus(registers[0] & 0x0000003F);
		slotCounterA = registers[4] & 0x0000FFFF;
		slotCounterB = (registers[4] & 0xFFFF0000) >> 16;
		rateCorrection = registers[6];
		offsetCorrection = registers[7];
	}

	decoded = true;
}