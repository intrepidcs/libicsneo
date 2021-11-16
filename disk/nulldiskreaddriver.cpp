#include "icsneo/disk/nulldiskreaddriver.h"

using namespace icsneo;

optional<uint64_t> NullDiskReadDriver::readLogicalDisk(Communication&, device_eventhandler_t report,
	uint64_t, uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return nullopt;
}

optional<uint64_t> NullDiskReadDriver::readLogicalDiskAligned(Communication&, device_eventhandler_t report,
	uint64_t, uint8_t*, uint64_t, std::chrono::milliseconds) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return nullopt;
}