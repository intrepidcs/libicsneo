#ifndef __ICSNEOCPP_H_
#define __ICSNEOCPP_H_

#include <vector>
#include <memory>

#include "icsneo/device/device.h"
#include "icsneo/api/version.h"

namespace icsneo {

std::vector<std::shared_ptr<Device>> FindAllDevices();

}

#endif