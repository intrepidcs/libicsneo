#ifndef __SCRIPTSTATUSMESSAGE_H
#define __SCRIPTSTATUSMESSAGE_H

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/packet/scriptstatuspacket.h"

namespace icsneo
{
//Response to Command::ScriptStatus
class ScriptStatusMessage : public Message {
public:
	ScriptStatusMessage() : Message( Message::Type::ScriptStatus ) {}

	bool isCoreminiRunning;
	uint32_t sectorOverflows;
	uint32_t numRemainingSectorBuffers;
	int32_t lastSector;
	int32_t readBinSize;
	int32_t minSector;
	int32_t maxSector;
	int32_t currentSector;
	uint64_t coreminiCreateTime;
	uint16_t fileChecksum;
	uint16_t coreminiVersion;
	uint16_t coreminiHeaderSize;
	uint8_t diagnosticErrorCode;
	uint8_t diagnosticErrorCodeCount;
	uint16_t maxCoreminiSizeKB;
};
}

#endif // __cplusplus

#endif //__SCRIPTSTATUSMESSAGE_H
