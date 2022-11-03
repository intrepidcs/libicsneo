#include <iostream>
#include "icsneo/communication/packet/scriptstatuspacket.h"
#include "icsneo/communication/message/scriptstatusmessage.h"

using namespace icsneo;

std::shared_ptr<ScriptStatusMessage> ScriptStatus::DecodeToMessage(const std::vector<uint8_t>& bytestream){
	if(bytestream.size() != sizeof(ScriptStatus))
		return {};

	auto msg = std::make_shared<ScriptStatusMessage>();
	const auto& decoded = *reinterpret_cast<const ScriptStatus*>(bytestream.data());
	msg->isCoreminiRunning = decoded.status.isRunning;
	msg->isEncrypted = decoded.status.isEncrypted;
	msg->sectorOverflows = decoded.sectorOverflows;
	msg->numRemainingSectorBuffers = decoded.numRemainingSectorBuffers;
	msg->lastSector = decoded.lastSector;
	msg->readBinSize = decoded.readBinSize;
	msg->minSector = decoded.minSector;
	msg->maxSector = decoded.maxSector;
	msg->currentSector = decoded.currentSector;
	msg->coreminiCreateTime = ((uint64_t)decoded.coreminiCreateTimeMsb << 32) | decoded.coreminiCreateTimeLsb;
	msg->fileChecksum = decoded.fileChecksum;
	msg->coreminiVersion = decoded.coreminiVersion;
	msg->coreminiHeaderSize = decoded.coreminiHeaderSize;
	msg->diagnosticErrorCode = decoded.diagErrCode;
	msg->diagnosticErrorCodeCount = decoded.diagErrCodeCount;
	msg->maxCoreminiSizeKB = decoded.maxCoreminiSizeKB;

	return msg;
}