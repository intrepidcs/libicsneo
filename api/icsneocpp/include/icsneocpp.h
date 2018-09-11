#ifndef __ICSNEOCPP_H_
#define __ICSNEOCPP_H_

#include <vector>
#include <memory>

#include "device/include/device.h"

namespace icsneo {
	std::vector<std::shared_ptr<Device>> FindAllDevices();
};

#endif