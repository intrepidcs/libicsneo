#ifndef __RADMOONT1SSETTINGS_H_
#define __RADMOONT1SSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	uint16_t network_enabled_on_boot;
	uint64_t network_enables;
	uint64_t network_enables_2;
	RAD_REPORTING_SETTINGS reporting;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	// Ethernet 10/100/1000
	ETHERNET_SETTINGS2 ethernet;
	// Ethernet General
	AE_GENERAL_SETTINGS aeGen;
	// 10T1S
	ETHERNET_SETTINGS2 ethT1s;
	ETHERNET10T1S_SETTINGS t1s;
} radmoont1s_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADMoonT1SSettings : public IDeviceSettings {
public:
	RADMoonT1SSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radmoont1s_settings_t)) {}
};

}

#endif // __cplusplus

#endif // __RADMOONT1SSETTINGS_H_
