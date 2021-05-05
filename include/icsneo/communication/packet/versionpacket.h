#ifndef __VERSIONPACKET_H__
#define __VERSIONPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/versionmessage.h"
#include <cstdint>
#include <memory>

namespace icsneo {

struct HardwareVersionPacket {
	static std::shared_ptr<VersionMessage> DecodeMainToMessage(const std::vector<uint8_t>& bytestream);
	static std::shared_ptr<VersionMessage> DecodeSecondaryToMessage(const std::vector<uint8_t>& bytestream);
};

}

#endif // __cplusplus

#endif