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

	bool isCoreminiRunning = false;
	uint32_t sectorOverflows = 0;
	uint32_t numRemainingSectorBuffers = 0;
	int32_t lastSector = 0;
	int32_t readBinSize = 0;
	int32_t minSector = 0;
	int32_t maxSector = 0;
	int32_t currentSector = 0;
	uint64_t coreminiCreateTime = 0;
	uint16_t fileChecksum = 0;
	uint16_t coreminiVersion = 0;
	uint16_t coreminiHeaderSize = 0;
	uint8_t diagnosticErrorCode = 0;
	uint8_t diagnosticErrorCodeCount = 0;
	uint16_t maxCoreminiSizeKB = 0;
};
}

#endif // __cplusplus

#endif //__SCRIPTSTATUSMESSAGE_H
