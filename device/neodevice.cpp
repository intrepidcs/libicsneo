#include "icsneo/device/neodevice.h"
#include "icsneo/device/founddevice.h"
#include <cstring>

neodevice_t::neodevice_t() : device(nullptr), handle(0), type(0) {
	memset(serial, 0, sizeof(serial));
}

neodevice_t::neodevice_t(const icsneo::FoundDevice& found, devicetype_t inType)
	: device(nullptr), handle(found.handle), type(inType) {
	static_assert(sizeof(found.serial) == sizeof(serial), "Serial sizes should match!");
	memcpy(serial, found.serial, sizeof(serial));
}
