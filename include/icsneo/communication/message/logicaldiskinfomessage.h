#ifndef __LOGICALDISKINFOMESSAGE_H_
#define __LOGICALDISKINFOMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <cstdint>

namespace icsneo {

class LogicalDiskInfoMessage : public Message {
public:
	LogicalDiskInfoMessage(bool isConnected, uint32_t numSectors, uint32_t numHidden, uint32_t sectorSize) :
		Message(Message::Type::LogicalDiskInfo), connected(isConnected), sectors(numSectors),
		hiddenSectors(numHidden), bytesPerSector(sectorSize) {}
	
	const bool connected;
	const uint32_t sectors;
	const uint32_t hiddenSectors;
	const uint32_t bytesPerSector;

	uint64_t getReportedSize() const { return uint64_t(sectors) * bytesPerSector; }
};

}

#endif // __cplusplus

#endif