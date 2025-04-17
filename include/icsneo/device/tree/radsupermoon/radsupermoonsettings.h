#ifndef __RADSUPERMOONSETTINGS_H_
#define __RADSUPERMOONSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif // __cplusplus

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en;

	AE_GENERAL_SETTINGS aeGen;
	AE_SETTINGS ae_01;

	uint16_t network_enables;
	uint16_t network_enables_2;
	uint16_t network_enabled_on_boot;
	uint16_t network_enables_3;

	STextAPISettings text_api;

	uint16_t pc_com_mode;
	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	uint16_t hwComLatencyTestEn;

	ETHERNET_SETTINGS2 eth2;
} radsupermoon_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADSupermoonSettings : public IDeviceSettings {
public:
	RADSupermoonSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radsupermoon_settings_t)) {}
};

}

#endif // __cplusplus

#endif