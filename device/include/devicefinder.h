#ifndef __DEVICEFINDER_H_
#define __DEVICEFINDER_H_

#include "device/include/device.h"
#include <vector>
#include <memory>

namespace icsneo {

class DeviceFinder {
public:
	static std::vector<std::shared_ptr<Device>> FindAll();
};

}

#endif