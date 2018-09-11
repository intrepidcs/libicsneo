#include <iterator>
#include <iostream>

#include "include/icsneocpp.h"
#include "device/include/devicefinder.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> icsneo::FindAllDevices() {
	return DeviceFinder::FindAll();
}