#ifndef __VALUECAN4_4_2EL_SETTINGS_H_
#define __VALUECAN4_4_2EL_SETTINGS_H_

#include "device/include/idevicesettings.h"
#include "device/valuecan4/settings/include/valuecan4settings.h"

#ifdef __cplusplus

namespace icsneo {

class ValueCAN4_4_2ELSettings : public IDeviceSettings {
public:
	ValueCAN4_4_2ELSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(valuecan4_4_2el_settings_t)) {}
	// We do not override getCANSettingsFor, getCANFDSettingsFor, or getEthernetSettingsFor here because they will be device specific
};

}

#endif // __cplusplus

#endif