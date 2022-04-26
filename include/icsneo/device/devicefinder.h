#ifndef __DEVICEFINDER_H_
#define __DEVICEFINDER_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include <vector>
#include <memory>

namespace icsneo {

class DeviceFinder {
public:
	static std::vector<std::shared_ptr<Device>> FindAll();
	static const std::vector<DeviceType>& GetSupportedDevices();
};

}

#endif // __cplusplus

#endif