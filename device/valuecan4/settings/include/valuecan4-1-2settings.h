#ifndef __VALUECAN4_1_2_SETTINGS_H_
#define __VALUECAN4_1_2_SETTINGS_H_

#include "device/include/idevicesettings.h"
#include "device/valuecan4/settings/include/valuecan4settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_1_2Settings : public IDeviceSettings {
public:
	ValueCAN4_1_2Settings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(valuecan4_1_2_settings_t)) {}
	// We do not override getCANSettingsFor or getCANFDSettingsFor here because they will be device specific
};

}

#endif // __cplusplus

#endif