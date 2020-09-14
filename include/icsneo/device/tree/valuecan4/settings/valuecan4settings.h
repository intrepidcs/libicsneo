#ifndef __VALUECAN4_SETTINGS_H_
#define __VALUECAN4_SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

// This is where the actual settings structures for all the ValueCAN 4 line live
// ValueCAN 4-1 and 4-2 share a structure, and 4-4 shares with 4-2EL, Industrial has its own

#pragma pack(push, 2)
typedef struct {
	/* Performance Test */
	uint16_t perf_en;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;

	uint64_t network_enables;
	uint64_t termination_enables;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	/* ISO15765-2 Transport Layer */
	int16_t iso15765_separation_time_offset;

	STextAPISettings text_api;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;
} valuecan4_1_2_settings_t, valuecan4_1_settings_t, valuecan4_2_settings_t;

typedef struct {
	uint16_t perf_en;
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	CAN_SETTINGS can3;
	CANFD_SETTINGS canfd3;
	CAN_SETTINGS can4;
	CANFD_SETTINGS canfd4;
	uint16_t network_enables;
	uint16_t network_enables_2;
	LIN_SETTINGS lin1;
	uint16_t network_enabled_on_boot;
	int16_t iso15765_separation_time_offset;
	uint16_t iso_9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;
	uint16_t iso_msg_termination_1;
	uint16_t network_enables_3;
	STextAPISettings text_api;
	uint64_t termination_enables;
	ETHERNET_SETTINGS ethernet;
	struct
	{
		uint32_t enableLatencyTest : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 30;
	} flags;
	uint16_t pwr_man_enable;
	uint16_t pwr_man_timeout;
} valuecan4_4_2el_settings_t, valuecan4_4_settings_t, valuecan4_2el_settings_t;

typedef struct {
	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	ETHERNET_SETTINGS ethernet;
	LIN_SETTINGS lin1;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;
	uint16_t iso_parity;
	uint16_t iso_msg_termination;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	uint16_t perf_en;
	int16_t iso15765_separation_time_offset;
	uint16_t network_enabled_on_boot;

	union {
		uint64_t word;
		struct
		{
			uint16_t network_enables;
			uint16_t network_enables_2;
			uint16_t network_enables_3;
		};
	} network_enables;
	uint64_t termination_enables;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t busMessagesToAndroid : 1;
		uint32_t enablePcEthernetComm : 1;
		uint32_t reserved : 28;
	} flags;
} valuecan4_industrial_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(valuecan4_1_2_settings_t) == 148, "ValueCAN 4-1 / 4-2 Settings are not packed correctly!");
static_assert(sizeof(valuecan4_4_2el_settings_t) == 326, "ValueCAN 4-4 / 4-2EL Settings are not packed correctly!");
static_assert(sizeof(valuecan4_industrial_settings_t) == 212, "ValueCAN 4 Industrial Settings are not packed correctly!");

} // End of namespace

#endif // __cplusplus

#endif