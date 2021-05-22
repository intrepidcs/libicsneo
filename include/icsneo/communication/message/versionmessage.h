#ifndef __VERSIONMESSAGE_H_
#define __VERSIONMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/device/deviceversion.h"
#include "icsneo/platform/optional.h"

namespace icsneo {

class VersionMessage : public Message {
public:
	enum Chip : uint8_t {
		MainChip,
		SecondaryChips
	};

	VersionMessage(Chip chip) : Message(Message::Type::DeviceVersion), ForChip(chip) {}

	// nullopt here indicates invalid
	std::vector< optional<DeviceAppVersion> > Versions;

	// What chips the versions are for
	const Chip ForChip;
};

}

#endif // __cplusplus

#endif