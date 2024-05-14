#ifndef _HARDWARE_INFO_MESSAGE_H_
#define _HARDWARE_INFO_MESSAGE_H_

#ifdef __cplusplus
#include "icsneo/communication/message/message.h"

namespace icsneo {

class HardwareInfo : public Message {
public:
	HardwareInfo() : Message(Message::Type::HardwareInfo) {}
	struct Version {
		uint8_t major;
		uint8_t minor;
	};

	struct Date {
		uint8_t day;
		uint8_t month;
		uint16_t year;
	};

	Date manufactureDate;
	Version hardwareRevision;
	Version bootloaderVersion;
};

}

#endif // __cplusplus

#endif