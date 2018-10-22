#include <iterator>
#include <iostream>

#include "icsneo/icsneocpp.h"
#include "icsneo/device/devicefinder.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> icsneo::FindAllDevices() {
	return DeviceFinder::FindAll();
}