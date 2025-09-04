#ifndef __FLEXRAYCLUSTER_H_
#define __FLEXRAYCLUSTER_H_

#ifdef __cplusplus

#include <cstdint>
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/device/extensions/flexray/erayregister.h"
#include "icsneo/device/extensions/flexray/poccommand.h"

namespace icsneo {

namespace FlexRay {

namespace Cluster {

#endif // __cplusplus

typedef enum {
	FLEXRAY_BAUDRATE_10M = 0,
	FLEXRAY_BAUDRATE_5M = 1, // 5Mbit currently not supported in the monitor
	FLEXRAY_BAUDRATE_2M5 = 2, // 2.5Mbit currently not supported in the monitor
	FLEXRAY_BAUDRATE_2M5_ALT = 3 // Per the ERAY spec, 0b11 also means 2.5MBits
} SpeedType;

typedef enum {
	FLEXRAY_SPP_5 = 0, // FlexRay 2.1 requires this value to be 0
	FLEXRAY_SPP_4 = 1,
	FLEXRAY_SPP_6 = 2,
	FLEXRAY_SPP_5_ALT = 3 // Per the ERAY spec, 0b11 also means sample point 5
} SPPType;

#ifdef __cplusplus

#define INIT(x) = x

#define neoflexray_cluster_config_t icsneo::FlexRay::Cluster::Configuration
struct Configuration {

#else // __cplusplus

#include <stdbool.h>
#include <stdint.h>

#define INIT(x)

typedef struct {

#endif // __cplusplus

	SpeedType Speed INIT(FLEXRAY_BAUDRATE_10M /* = 0 */); // gdSampleClockPeriod, pSamplesPerMicrotick, Baud Rate Prescaler
	SPPType StrobePointPosition INIT(FLEXRAY_SPP_5 /* = 0 */);

	uint8_t ActionPointOffset INIT(0); // gdActionPointOffset
	uint8_t CASRxLowMax INIT(0); // gdCASRxLowMax
	uint8_t ColdStartAttempts INIT(0); // gColdStartAttempts
	uint16_t CycleDurationMicroSec INIT(0); // gdCycle
	uint8_t DynamicSlotIdlePhaseMinislots INIT(0); // gdDynamicSlotIdlePhase, In Minislot increments [0..2]
	uint8_t ListenNoiseMacroticks INIT(0); // gListenNoise
	uint16_t MacroticksPerCycle INIT(0); // gMacroPerCycle
	uint8_t MacrotickDurationMicroSec INIT(0); // gdMacroTick
	uint8_t MaxWithoutClockCorrectionFatal INIT(0); // gMaxWithoutClockCorrectionFatal
	uint8_t MaxWithoutClockCorrectionPassive INIT(0); // gMaxWithoutClockCorrectionPassive
	uint8_t MinislotActionPointOffsetMacroticks INIT(0); // gdMinislotActionPointOffset
	uint8_t MinislotDurationMacroticks INIT(0); // gdMinislot
	uint16_t NetworkIdleTimeMacroticks INIT(0); // gdNIT
	uint8_t NetworkManagementVectorLengthBytes INIT(0); // gNetworkManagementVectorLength
	uint16_t NumberOfMinislots INIT(0); // gNumberOfMinislots
	uint16_t NumberOfStaticSlots INIT(0); // gNumberOfStaticSlots
	uint16_t OffsetCorrectionStartMacroticks INIT(0); // gOffsetCorrectionStart
	uint8_t PayloadLengthOfStaticSlotInWords INIT(0); // gPayloadLengthStatic

	uint16_t StaticSlotMacroticks INIT(0); // gdStaticSlot
	uint32_t SymbolWindowMacroticks INIT(0); // gdSymbolWindow
	uint32_t SymbolWindowActionPointOffsetMacroticks INIT(0);
	uint8_t SyncFrameIDCountMax INIT(0); // gSyncNodeMax

	uint8_t TransmissionStartSequenceDurationBits INIT(0); // gdTSSTransmitter
	uint8_t WakeupRxIdleBits INIT(0); // gdWakeupSymbolRxIdle
	uint8_t WakeupRxLowBits INIT(0); // gdWakeupSymbolRxLow
	uint16_t WakeupRxWindowBits INIT(0); // gdWakeupSymbolRxWindow
	uint8_t WakeupTxActiveBits INIT(0); // gdWakeupSymbolTxLow (active = low here)
	uint8_t WakeupTxIdleBits INIT(0); // gdWakeupSymbolTxIdle

#ifdef __cplusplus
};
#else
} neoflexray_cluster_config_t;
#endif

#ifdef __cplusplus
} // namespace Cluster

} // namespace FlexRay

} // namespace icsneo
#endif // __cplusplus

#endif // __FLEXRAYCLUSTER_H_