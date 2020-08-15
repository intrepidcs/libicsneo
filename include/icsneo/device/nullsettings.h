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
	NullSettings(std::shared_ptr<Communication> com) : IDeviceSettings(nullptr, com) {}
};

}

#endif // __cplusplus

#endif