#ifndef __NULL_SETTINGS_H_
#define __NULL_SETTINGS_H_

#ifdef __cplusplus

#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/communication.h"
#include <memory>

namespace icsneo {

class NullSettings : public IDeviceSettings {
public:
	// Calls the protected base constructor with "createInoperableSettings"
	NullSettings(Device* device) : IDeviceSettings(nullptr, device) {}
};

}

#endif // __cplusplus

#endif