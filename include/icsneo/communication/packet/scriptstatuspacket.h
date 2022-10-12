#ifndef __SCRIPTSTATUSPACKET_H
#define __SCRIPTSTATUSPACKET_H

#ifdef __cplusplus

#include "icsneo/communication/packet.h"
#include <cstdint>
#include <memory>

#pragma pack(push,2)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable: 4200) // nonstandard extension used: zero-sized array in struct/union
#endif // _MSC_VER

namespace icsneo
{

class ScriptStatusMessage;

struct CoreMiniStatus
{
	uint32_t justReset : 1;
	uint32_t communicationEnabled : 1;
	uint32_t isRunning : 1;
	uint32_t checksumFailed : 1;
	uint32_t licenseFailed : 1;
	uint32_t versionMismatch : 1;
	uint32_t bootOff : 1;
	uint32_t hardwareFailure : 1; // To check SRAM failure (for now)
	uint32_t isPassiveConnect : 1; // Always zero. Set to one when neoVI connection is passive,i.e. no async traffic
	uint32_t usbCommunicationEnabled : 1; // Set to one when USB Host PC has enabled communication.
	uint32_t linuxCommunicationEnabled : 1; // Set to one when Android (Linux) has enabled communication.
	uint32_t tooBig : 1;
	uint32_t hidUsbState : 1;
	uint32_t fpgaUsbState : 1;
	uint32_t filesystem : 1;
	uint32_t isEncrypted : 1;
	uint32_t reserved : 16;
};

struct ScriptStatus
{
	static std::shared_ptr<ScriptStatusMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	CoreMiniStatus status;
	uint32_t sectorOverflows;
	uint32_t numRemainingSectorBuffers;
	int32_t lastSector;
	int32_t readBinSize;
	int32_t minSector;
	int32_t maxSector;
	int32_t currentSector;
	uint32_t coreminiCreateTimeMsb;
	uint32_t coreminiCreateTimeLsb;
	uint16_t zero2;
	uint16_t zero3;
	uint16_t fileChecksum;
	uint16_t coreminiVersion;
	uint16_t coreminiHeaderSize;
	uint16_t coreminiEngineMinVersion; // This firmware min version
	uint16_t coreminiScriptMinEngineVers; // Current loaded script's min engine requirements.
	uint8_t eolResults1;
	uint8_t eolResults2;
	uint8_t eolResults3;
	uint8_t zero6;
	uint8_t diagErrCode;
	uint8_t diagErrCodeCount;
	/* FPGA version Fpga reports to HID over SPI */
	uint8_t spiFpgaVers1;
	uint8_t spiFpgaVers2;
	uint16_t maxCoreminiSizeKB;
	uint16_t maxCoreminiFlashSizeKB;
};

} // namespace icsneo

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
#pragma pack(pop)

#endif // __cplusplus

#endif //__SCRIPTSTATUSPACKET_H
