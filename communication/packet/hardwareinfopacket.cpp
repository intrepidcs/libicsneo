#include "icsneo/communication/packet/hardwareinfopacket.h"
#include "icsneo/communication/message/hardwareinfo.h"
#include <iostream>
using namespace icsneo;

#pragma pack(push, 1)
typedef struct
{
	uint8_t valid;
	struct
	{
		uint8_t day;
		uint8_t month;
		uint16_t year;
	} manufactureDate;
	struct
	{
		uint8_t major;
		uint8_t minor;
	} hwRev;
	uint8_t deviceId;
	struct
	{
		uint8_t major;
		uint8_t minor;
	} blVersion;
} HardwareInfoFrame;
#pragma pack(pop)

std::shared_ptr<HardwareInfo> HardwareInfoPacket::DecodeToMessage(const std::vector<uint8_t>& bytes) {
	if(bytes.size() < (sizeof(HardwareInfoFrame) + 1)) {
		return nullptr;
	}

	const auto* frame = reinterpret_cast<const HardwareInfoFrame*>(&bytes[1]);
	
	auto msg = std::make_shared<HardwareInfo>();

	msg->manufactureDate.day = frame->manufactureDate.day;
	msg->manufactureDate.year = frame->manufactureDate.year;
	msg->manufactureDate.month = frame->manufactureDate.month;

	msg->hardwareRevision.major = frame->hwRev.major;
	msg->hardwareRevision.minor = frame->hwRev.minor;

	msg->bootloaderVersion.major = frame->blVersion.major;
	msg->bootloaderVersion.minor = frame->blVersion.minor;

	return msg;
}
