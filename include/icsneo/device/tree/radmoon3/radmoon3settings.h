#ifndef __RADMOON3SETTINGS_H_
#define __RADMOON3SETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif // __cplusplus

#pragma pack(push, 2)
typedef struct {
	uint16_t perf_en; // 2

	ETHERNET10G_SETTINGS autoEth10g; // 24
	ETHERNET10G_SETTINGS eth10g; // 24

	uint16_t network_enables; // 2
	uint16_t network_enables_2; // 2
	uint16_t network_enabled_on_boot; // 2
	uint16_t network_enables_3; // 2

	struct
	{
		uint16_t enableLatencyTest : 1;
		uint16_t reserved : 15;
	} flags; // 2
	uint64_t network_enables_5;
} radmoon3_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

#include <iostream>

class RADMoon3Settings : public IDeviceSettings {
public:
	RADMoon3Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radmoon3_settings_t)) {}
};

}

#endif // __cplusplus

#endif