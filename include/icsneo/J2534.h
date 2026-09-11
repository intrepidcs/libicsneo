#ifndef _J2534_H
#define _J2534_H

#include <cstdint>

namespace icsneo {

//J2534 Commands
#define J1534_NUM_PERIOD_TX_MSGS 128
#define J1534_NUM_PERIOD_RX_MSGS 128

enum class J2534Command : uint8_t {
    SetISJ2534 = 0,
    SetISO5Baud = 1,
    SetISOFastInit = 2,
    SetISOCheckSum = 3,
    SetISO9141Parms = 4,
    GetISO9141Parms = 5,
    ISO9141APIChkSum = 6,
    SetNetworkBaudRate = 7,
    GetNetworkBaudRate = 8,
    EnableTransmitEvent = 9,
    SetTransmitEvent = 10,
    BlueEnableStopFilters = 11,
    Blue15765HWSupport = 12,
    GetTXBufferInfo = 13,
    GetEncryptionKey = 14,
    SetMiscIOForVBATT = 15,
    EnableISO_KW_Network = 16,
    SetJ1708CheckSum = 17,
    GetTimestamp = 18,
    GetCANFDRate = 19,
    SetCANFDRate = 20,
    GetCANFDTermination = 21,
    SetCANFDTermination = 22,
    GetCANFDFormat = 23,
    SetCANFDFormat = 24
};

#pragma pack(push)
#pragma pack(1)

/**
 * SubFrame Commands
 */
enum class J2534SubCommand : uint8_t {
    Enable = 0,
    SetupIso15765RxFilter = 1,
    SetupIso15765TxMessageProperties = 2,
    SetupIso15765TxMessageDataBytes = 3,
    SetupCanRxFilter = 4,
    SetupClearCanRxFilters = 5,
    EnableFiltering = 6
};

typedef struct
{
	uint16_t uiFilterMask0;
	uint16_t uiFilterID0;
	uint16_t uiFilterMask1;
	uint16_t uiFilterID1;
	uint16_t uiFilterMask2;
	uint16_t uiFilterID2;
	union {
		/** Mask and value words if rx filter is datalink filter*/
		struct
		{
			uint16_t uiFilterMask3;
			uint16_t uiFilterID3;
			uint16_t uiFilterMask4;
			uint16_t uiFilterID4;
			uint16_t uiFilterMask5;
			uint16_t uiFilterID5;
			uint16_t uiFilterMask6;
			uint16_t uiFilterID6;
		} datalink;
		/** Mask and value bytes if rx filter is datalink filter*/
		struct
		{
			uint8_t uiFilterMask_DB0;
			uint8_t uiFilterMask_DB1;
			uint8_t uiFilterID_DB0;
			uint8_t uiFilterID_DB1;
			uint8_t uiFilterMask_DB2;
			uint8_t uiFilterMask_DB3;
			uint8_t uiFilterID_DB2;
			uint8_t uiFilterID_DB3;
			uint8_t uiFilterMask_DB4;
			uint8_t uiFilterMask_DB5;
			uint8_t uiFilterID_DB4;
			uint8_t uiFilterID_DB5;
			uint8_t uiFilterMask_DB6;
			uint8_t uiFilterMask_DB7;
			uint8_t uiFilterID_DB6;
			uint8_t uiFilterID_DB7;
		} datalinkBytes;
		/** Mask and value bytes if rx filter is Iso15765-2*/
		struct
		{
			uint8_t uiFilterMask_DB0;
			uint8_t uiFilterID_DB0;
			uint8_t uiFilterMask_DB1;
			uint8_t uiFilterID_DB1;
			uint8_t uiFilterMask_DB2;
			uint8_t uiFilterID_DB2;
			uint8_t uiFilterMask_DB3;
			uint8_t uiFilterID_DB3;
			uint8_t uiFilterMask_DB4;
			uint8_t uiFilterID_DB4;
			uint8_t uiFilterMask_DB5;
			uint8_t uiFilterID_DB5;
			uint8_t uiFilterMask_DB6;
			uint8_t uiFilterID_DB6;
			uint8_t uiFilterMask_DB7;
			uint8_t uiFilterID_DB7;
		} iso15Bytes;
	} args;
} MessageFilterBytes;

typedef struct
{
	uint16_t idx;
	uint16_t enabled;
	uint16_t filterCount;
	MessageFilterBytes fb;
	uint16_t networkId;
} J2534_RxCanFilter;

#pragma pack(pop)

} // namespace icsneo

#endif
