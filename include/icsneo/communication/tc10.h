#ifndef __ICSNEO_TC10_H_
#define __ICSNEO_TC10_H_

#ifdef __cplusplus

#include <cstdint>
#include "icsneo/icsneoc2types.h"

namespace icsneo {

enum class TC10WakeStatus : icsneoc2_tc10_wake_status_t {
	NoWakeReceived = icsneoc2_tc10_wake_status_no_wake_received,
	WakeReceived = icsneoc2_tc10_wake_status_wake_received,
};

enum class TC10SleepStatus : icsneoc2_tc10_sleep_status_t {
	NoSleepReceived = icsneoc2_tc10_sleep_status_no_sleep_received,
	SleepReceived = icsneoc2_tc10_sleep_status_sleep_received,
	SleepFailed = icsneoc2_tc10_sleep_status_sleep_failed,
	SleepAborted = icsneoc2_tc10_sleep_status_sleep_aborted,
};

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_

typedef enum _neotc10wakestatus_t {
	ICSNEO_TC10_NO_WAKE_RECEIVED = 0,
	ICSNEO_TC10_WAKE_RECEIVED = 1,
} neotc10wakestatus_t;

typedef enum _neotc10sleepstatus_t {
	ICSNEO_TC10_NO_SLEEP_RECEIVED = 0,
	ICSNEO_TC10_SLEEP_RECEIVED = 1,
	ICSNEO_TC10_SLEEP_FAILED = 2,
	ICSNEO_TC10_SLEEP_ABORTED = 3,
} neotc10sleepstatus_t;

typedef struct _neotc10status_t {
	neotc10wakestatus_t wakeStatus;
	neotc10sleepstatus_t sleepStatus;
} neotc10status_t;

#endif

#endif // __ICSNEO_TC10_H_
