#ifndef __FLEXRAYPOCSTATUS_H_
#define __FLEXRAYPOCSTATUS_H_

#ifdef __cplusplus

#include <cstdint>

namespace icsneo {

namespace FlexRay {

enum class POCStatus : uint8_t {
	DefaultConfig = 0x00,
	Ready = 0x01,
	NormalActive = 0x02,
	NormalPassive = 0x03,
	Halt = 0x04,
	MonitorMode = 0x05,
	Config = 0x0f,
	WakeupStandby = 0x10,
	WakeupListen = 0x11,
	WakeupSend = 0x12,
	WakeupDetect = 0x13,
	StartupPrepare = 0x20,
	ColdstartListen = 0x21,
	ColdstartCollisionResolution = 0x22,
	ColdstartConsistencyCheck = 0x23,
	ColdstartGap = 0x24,
	ColdstartJoin = 0x25,
	IntegrationColdstartCheck = 0x26,
	IntegrationListen = 0x27,
	IntegrationConsistencyCheck = 0x28,
	InitializeSchedule = 0x29,
	AbortStartup = 0x2a,
	StartupSuccess = 0x2b,
	Unknown = 0xff
};

} // namespace FlexRay

} // namespace icsneo

#endif // __cplusplus

#endif // __FLEXRAYPOCSTATUS_H_