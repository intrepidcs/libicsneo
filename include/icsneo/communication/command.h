#ifndef __COMMAND_H_
#define __COMMAND_H_

#ifdef __cplusplus

namespace icsneo {

enum class Command : uint8_t {
	EnableNetworkCommunication = 0x07,
	EnableNetworkCommunicationEx = 0x08,
	NeoReadMemory = 0x40,
	NeoWriteMemory = 0x41,
	ClearCoreMini = 0x42,
	LoadCoreMini = 0x43,
	GetRTC = 0x49,
	SetRTC = 0x50,
	RequestSerialNumber = 0xA1,
	GetMainVersion = 0xA3, // Previously known as RED_CMD_APP_VERSION_REQ
	SetSettings = 0xA4, // Previously known as RED_CMD_SET_BAUD_REQ, follow up with SaveSettings to write to EEPROM
	//GetSettings = 0xA5, // Previously known as RED_CMD_READ_BAUD_REQ, now unused
	SaveSettings = 0xA6,
	UpdateLEDState = 0xA7,
	SetDefaultSettings = 0xA8, // Follow up with SaveSettings to write to EEPROM
	GetSecondaryVersions = 0xA9, // Previously known as RED_CMD_PERIPHERALS_APP_VERSION_REQ, versions other than the main chip
	GetHardwareInfo = 0xB7, // Previously known as RED_CMD_HARDWARE_VERSION_REQ
	GetLogicalDiskInfo = 0xBB, // Previously known as RED_CMD_GET_SDCARD_INFO
	RequestStatusUpdate = 0xBC,
	ReadSettings = 0xC7, // Previously known as 3G_READ_SETTINGS_EX
	SetVBattMonitor = 0xDB, // Previously known as RED_CMD_CM_VBATT_MONITOR
	RequestBitSmash = 0xDC, // Previously known as RED_CMD_CM_BITSMASH
	WiVICommand = 0xDD, // Previously known as RED_CMD_WIVI_COMM
	GetVBattReq = 0xDF, // Previously known as RED_CMD_VBATT_REQUEST
	ScriptStatus = 0xE0, // Previously known as RED_CMD_SCRIPT_STATUS
	MiscControl = 0xE7,
	NeoEraseMemory = 0xEA, // Previously known as RED_CMD_ERASE_MEMORY
	Extended = 0xF0, // Previously known as RED_CMD_EXT_COMM
	ExtendedData = 0xF2, // Previously known as RED_CMD_EXTENDED_DATA
	FlexRayControl = 0xF3,
	CoreMiniPreload = 0xF4, // Previously known as RED_CMD_COREMINI_PRELOAD
	PHYControlRegisters = 0xEF,
};

enum class ExtendedCommand : uint16_t {
	GenericReturn = 0x0000,
	GetDiskDetails = 0x0010,
	DiskFormatStart = 0x0011,
	DiskFormatCancel = 0x0012,
	DiskFormatProgress = 0x0013,
	DiskFormatUpdate = 0x0014,
	Extract = 0x0015,
	StartDHCPServer = 0x0016,
	StopDHCPServer = 0x0017,
	GetSupportedFeatures = 0x0018,
	GetGPTPStatus = 0x0019,
	GetComponentVersions = 0x001A,
	Reboot = 0x001C,
	SetRootFSEntryFlags = 0x0027,
	GenericBinaryInfo = 0x0030,
	LiveData = 0x0035,
	RequestTC10Wake = 0x003D,
	RequestTC10Sleep = 0x003E,
	GetTC10Status = 0x003F,
};

enum class ExtendedResponse : int32_t {
	OK = 0,
	InvalidCommand = -1,
	InvalidState = -2,
	OperationFailed = -3,
	OperationPending = -4,
	InvalidParameter = -5,
};

enum class ExtendedDataSubCommand : uint32_t {
	GenericBinaryRead = 13,
};

#pragma pack(push,1)
struct ExtendedCommandHeader {
	uint8_t netid; // should be Main51
	uint16_t fullLength;
	uint8_t command; // should be Command::Extended
	uint16_t extendedCommand;
	uint16_t payloadLength;
};
#pragma pack(pop)

}

#endif // __cplusplus

#endif