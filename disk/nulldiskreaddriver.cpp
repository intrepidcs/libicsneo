#include "icsneo/disk/nulldiskreaddriver.h"

using namespace icsneo;

optional<uint64_t> NullDiskReadDriver::readLogicalDisk(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}

optional<uint64_t> NullDiskReadDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	report(APIEvent::Type::DiskNotSupported, APIEvent::Severity::Error);
	return std::nullopt;
}