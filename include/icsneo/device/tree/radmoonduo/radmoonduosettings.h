#ifndef __RADMOONDUOSETTINGS_H_
#define __RADMOONDUOSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif // __cplusplus

#pragma pack(push, 2)
enum RadMoonDuoConverterMode {
	CONVERTER_TARGET_RJ45 = 0,
	CONVERTER_TARGET_USB_CM
};

typedef struct {
	// AE_LINK_AUTO/MASTER/SLAVE
	uint8_t linkMode0;
	uint8_t linkMode1;
	// USB/CM or RJ45 selection (RadMoonDuoConverterMode)
	uint8_t converter1Mode;
	uint8_t reserved;
	// IP Settings if converter is hooked up to Coremini
	// Currently unused
	uint32_t ipAddress;
	uint32_t ipMask;
	uint32_t ipGateway;
} RadMoonDuoConverterSettings;

typedef struct {
	uint16_t perf_en;
	uint16_t network_enabled_on_boot;
	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;
	RadMoonDuoConverterSettings converter;
	uint64_t network_enables;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;
} radmoonduo_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(RadMoonDuoConverterSettings) == 16, "RAD-Moon Duo converter settings size error");
static_assert(sizeof(radmoonduo_settings_t) == 38, "RAD-Moon Duo settings size error");

#include <iostream>

class RADMoonDuoSettings : public IDeviceSettings {
public:
	RADMoonDuoSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radmoonduo_settings_t)) {}
};

}

#endif // __cplusplus

#endif