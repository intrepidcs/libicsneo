#include "icsneo/disk/nulldiskdriver.h"

using namespace icsneo;
using namespace icsneo::Disk;

std::optional<uint64_t> NullDriver::readLogicalDisk(Communication&, device_eventhandler_t report,
	uint64_t, uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}

std::optional<uint64_t> NullDriver::readLogicalDiskAligned(Communication&, device_eventhandler_t report,
	uint64_t, uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}

std::optional<uint64_t> NullDriver::writeLogicalDisk(Communication&, device_eventhandler_t report, ReadDriver&,
	uint64_t, const uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}

std::optional<uint64_t> NullDriver::writeLogicalDiskAligned(Communication&, device_eventhandler_t report,
	uint64_t, const uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}