#include <icsneo/disk/diskdetails.h>
#include <icsneo/device/idevicesettings.h>

using namespace icsneo;

#pragma pack(push, 2)

struct DISK_STATUS {
	uint16_t status;
	uint64_t sectors;
	uint32_t bytesPerSector;
};

struct DISK_DETAILS {
	DISK_SETTINGS settings;
	uint16_t options;
	DISK_STATUS status[12];
};

#pragma pack(pop)

static constexpr uint8_t DISK_FORMAT_FAT32 = 0x01u;
static constexpr uint8_t DISK_STATUS_PRESENT = 0x01u;
static constexpr uint8_t DISK_STATUS_INITIALIZED = 0x02u;
static constexpr uint16_t DISK_DETAILS_FULL_FORMAT = 0x01u;

std::vector<uint8_t> DiskDetails::Encode(const DiskDetails& config) {
	std::vector<uint8_t> result(sizeof(DISK_DETAILS), 0);
	DISK_DETAILS* details = reinterpret_cast<DISK_DETAILS*>(result.data());
	details->settings.disk_layout = static_cast<uint8_t>(config.layout);
	details->settings.disk_format = DISK_FORMAT_FAT32; // Always FAT32
	details->options = config.fullFormat ? DISK_DETAILS_FULL_FORMAT : 0;
	uint32_t& enables = details->settings.disk_enables;
	enables = 0u;
	for(size_t i = 0; i < config.disks.size(); i++) {
		auto& disk = config.disks[i];
		details->status[i].sectors = disk.sectors;
		details->status[i].bytesPerSector = disk.bytesPerSector;

		uint16_t& status = details->status[i].status;
		status = 0u;
		if(disk.initialized) {
			status |= DISK_STATUS_INITIALIZED;
		}
		if(disk.present) {
			status |= DISK_STATUS_PRESENT;
		}
		if(disk.formatted) {
			enables |= (1u << i);
		}
	}

	return result;
}

std::shared_ptr<DiskDetails> DiskDetails::Decode(const std::vector<uint8_t>& bytes, size_t diskCount, device_eventhandler_t report) {
	if(bytes.size() < sizeof(DISK_DETAILS)) {
		report(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error);
		return nullptr;
	}

	auto result = std::make_shared<DiskDetails>();
	const DISK_DETAILS* details = reinterpret_cast<const DISK_DETAILS*>(bytes.data());
	result->layout = static_cast<DiskLayout>(details->settings.disk_layout);
	result->fullFormat = details->options & DISK_DETAILS_FULL_FORMAT;

	result->disks.reserve(diskCount);

	for(size_t i = 0; i < diskCount; i++) {
		auto& disk = details->status[i];
		result->disks.emplace_back();
		auto& info = result->disks.back();
		info.present = static_cast<bool>(disk.status & DISK_STATUS_PRESENT);
		info.initialized = static_cast<bool>(disk.status & DISK_STATUS_INITIALIZED);
		info.formatted = static_cast<bool>(details->settings.disk_enables & (1u << i));  
		info.sectors = disk.sectors;
		info.bytesPerSector = disk.bytesPerSector;
	}

	return result;
}
