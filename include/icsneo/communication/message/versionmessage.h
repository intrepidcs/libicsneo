#ifndef __VERSIONMESSAGE_H_
#define __VERSIONMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/device/deviceversion.h"
#include "icsneo/platform/optional.h"

namespace icsneo {

class VersionMessage : public Message {
public:
	VersionMessage(bool main) : MainChip(main) { network = Network::NetID::Main51; }

	// If true, the included version is for the main chip
	const bool MainChip;

	// nullopt here indicates invalid
	std::vector< optional<DeviceAppVersion> > Versions;
};

}

#endif // __cplusplus

#endif