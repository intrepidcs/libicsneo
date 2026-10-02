#ifndef LIBICSNEO_RADARGOSSETTINGS_H
#define LIBICSNEO_RADARGOSSETTINGS_H

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif // __cplusplus

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en; // 2

	ETHERNET_SETTINGS2 ethernet1; // 16
	ETHERNET_SETTINGS2 ethernet2; // 16
	ETHERNET_SETTINGS2 autoEthernet1; // 16
	ETHERNET_SETTINGS2 autoEthernet2; // 16

	uint16_t network_enabled_on_boot; // 2
	uint16_t network_enables; // 2
	uint16_t network_enables_2; // 2
	uint16_t network_enables_3; // 2
	uint16_t network_enables_4; // 2
	uint64_t network_enables_5; // 8

	struct
	{
		uint16_t enableLatencyTest : 1;
		uint16_t reserved : 15;
	} flags; // 2
} radargos_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radargos_settings_t) == 86, "RADArgos settings size mismatch");

#include <iostream>

class RADArgosSettings : public IDeviceSettings {
public:
	RADArgosSettings(Device* device) : IDeviceSettings(device, sizeof(radargos_settings_t)) {}
};

}

#endif // __cplusplus


#endif // LIBICSNEO_RADARGOSSETTINGS_H
