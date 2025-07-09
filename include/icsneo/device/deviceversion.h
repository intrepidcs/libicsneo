#ifndef __DEVICEVERSION_H_
#define __DEVICEVERSION_H_

#ifdef __cplusplus

#include <optional>
#include <cstdint>
#include <array>

namespace icsneo {

struct DeviceAppVersion {
	uint8_t major = 0;
	uint8_t minor = 0;

	DeviceAppVersion() = default;
	DeviceAppVersion(const uint8_t major, const uint8_t minor)
		: major(major), minor(minor) {}

	bool operator!=(const DeviceAppVersion& rhs) const { return major != rhs.major || minor != rhs.minor; }
	bool operator==(const DeviceAppVersion& rhs) const { return major == rhs.major && minor == rhs.minor; }
};

} // namespace icsneo


#endif // __cplusplus

#endif