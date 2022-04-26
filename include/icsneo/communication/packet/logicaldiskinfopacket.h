#ifndef __LOGICALDISKINFOPACKET_H__
#define __LOGICALDISKINFOPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/logicaldiskinfomessage.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>

namespace icsneo {

#pragma pack(push,2)
struct LogicalDiskInfoPacket {
	static std::shared_ptr<LogicalDiskInfoMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	uint16_t isConnected;
	uint32_t numSectors;
	uint32_t hiddenSectors;
	uint32_t bytesPerSector;
};
#pragma pack(pop)

}

#endif // __cplusplus

#endif