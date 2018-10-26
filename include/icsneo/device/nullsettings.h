#ifndef __NULL_SETTINGS_H_
#define __NULL_SETTINGS_H_

#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/communication.h"
#include <memory>

namespace icsneo {

class NullSettings : public IDeviceSettings {
public:
	// Calls the base constructor with "createInoperableSettings"
	NullSettings(std::shared_ptr<Communication> com = std::shared_ptr<Communication>()) : IDeviceSettings(nullptr) { (void)com; }
};

}

#endif