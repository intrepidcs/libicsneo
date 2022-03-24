#ifndef __FOUNDDEVICE_H_
#define __FOUNDDEVICE_H_

#include "icsneo/communication/driver.h"
#include "icsneo/device/neodevice.h"

namespace icsneo {

typedef std::function< std::unique_ptr<Driver>(device_eventhandler_t err, neodevice_t& forDevice) > driver_factory_t;

class FoundDevice {
public:
	neodevice_handle_t handle = 0;
	char serial[7] = {};
	uint16_t productId = 0;
	driver_factory_t makeDriver;
};

} // namespace icsneo

#endif // __FOUNDDEVICE_H_