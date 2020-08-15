#ifndef __FLEXRAYPOCCOMMAND_H_
#define __FLEXRAYPOCCOMMAND_H_

#ifdef __cplusplus

#include <cstdint>

namespace icsneo {

namespace FlexRay {

enum class POCCommand : uint8_t {
	CommandNotAccepted = 0x00,
	Config = 0x01,
	Ready = 0x02,
	Wakeup = 0x03,
	Run = 0x04,
	AllSlots = 0x05,
	Halt = 0x06,
	Freeze = 0x07,
	SendMTS = 0x08,
	AllowColdstart = 0x09,
	ResetStatusIndicators = 0x0a,
	MonitorMode = 0x0b,
	ClearRAMs = 0x0c,
	Unknown = 0xff
};

} // namespace FlexRay

} // namespace icsneo

#endif // __cplusplus

#endif // __FLEXRAYPOCCOMMAND_H_