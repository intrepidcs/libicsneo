#ifndef __FLEXRAYOPCODE_H_
#define __FLEXRAYOPCODE_H_

#include <cstdint>

namespace icsneo {

namespace FlexRay {

// Second byte, opcode
enum class Opcode : uint8_t {
	SetNetID = 0x00,
	ReadCCRegs = 0x01,
	WriteCCReg = 0x02,
	WriteMessageBuffer = 0x03,
	ReadCCStatus = 0x04,
	AddConfiguredMessage = 0x05,
	InitForRun = 0x06,
	Unknown = 0xff
};

} // namespace FlexRay

} // namespace icsneo

#endif // __FLEXRAYOPCODE_H_