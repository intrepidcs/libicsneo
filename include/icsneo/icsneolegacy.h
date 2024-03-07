#ifndef __ICSNEOLEGACY_H_
#define __ICSNEOLEGACY_H_

#include "icsneo/platform/dynamiclib.h" // Dynamic library loading and exporting
#include "icsneo/platform/tchar.h"

#include <stdint.h>
#include <stddef.h>
typedef uint8_t byte; // Typedef helper for the following include
#include "icsneo/icsnVC40.h" // Definitions for structs

// From coremini.h
#define MAX_BIT_SMASH_ARBIDS (4)
#define BIT_SMASH_OPTION_EXTENDED (1)
#define BIT_SMASH_OPTION_REMOTE (2)
#define BIT_SMASH_OPTION_BUSTEST (4)
#define BIT_SMASH_OPTION_WHITELIST (8)

#ifdef __cplusplus
extern "C" {
#endif

//Basic Functions
extern int LegacyDLLExport icsneoFindDevices(NeoDeviceEx* pNeoDeviceEx, int* pNumDevices, unsigned int* DeviceTypes, unsigned int numDeviceTypes,POptionsFindNeoEx* pOptionsNeoEx, unsigned int* reserved);
// icsneoFindNeoDevices was marked as obsolete in icsneo40 in 2016, Please move to icsneoFindDevices
// extern int LegacyDLLExport icsneoFindNeoDevices(unsigned long DeviceTypes, NeoDevice* pNeoDevice, int* pNumDevices);
// icsneoOpenNeoDevice was marked as obsolete in icsneo40 in 2016, Please move to icsneoOpenNeoDevice
// extern int LegacyDLLExport icsneoOpenNeoDevice(NeoDevice* pNeoDevice, void** hObject, unsigned char* bNetworkIDs, int bConfigRead, int bSyncToPC);
extern int LegacyDLLExport icsneoOpenDevice(NeoDeviceEx* pNeoDeviceEx, void** hObject, unsigned char* bNetworkIDs, int bConfigRead, int iOptions, OptionsOpenNeoEx* stOptionsOpenNeoEx, unsigned long reserved);
extern int LegacyDLLExport icsneoClosePort(void* hObject, int* pNumberOfErrors);
extern void LegacyDLLExport icsneoFreeObject(void* hObject);

//Message Functions
extern int LegacyDLLExport icsneoGetMessages(void* hObject, icsSpyMessage* pMsg, int* pNumberOfMessages, int* pNumberOfErrors);
extern int LegacyDLLExport icsneoTxMessages(void* hObject, icsSpyMessage* pMsg, int lNetworkID, int lNumMessages);
extern int LegacyDLLExport icsneoTxMessagesEx(void* hObject,icsSpyMessage* pMsg, unsigned int lNetworkID, unsigned int lNumMessages, unsigned int* NumTxed, unsigned int zero2);
extern int LegacyDLLExport icsneoWaitForRxMessagesWithTimeOut(void* hObject, unsigned int iTimeOut);
extern int LegacyDLLExport icsneoEnableNetworkRXQueue(void* hObject, int iEnable);
extern int LegacyDLLExport icsneoGetTimeStampForMsg(void* hObject, icsSpyMessage* pMsg, double* pTimeStamp);
extern void LegacyDLLExport icsneoGetISO15765Status(void* hObject, int lNetwork, int lClearTxStatus, int lClearRxStatus, int*lTxStatus, int*lRxStatus);
extern void LegacyDLLExport icsneoSetISO15765RxParameters(void* hObject, int lNetwork, int lEnable, spyFilterLong* pFF_CFMsgFilter, icsSpyMessage* pTxMsg,
										int lCFTimeOutMs, int lFlowCBlockSize, int lUsesExtendedAddressing, int lUseHardwareIfPresent);

//RTC Functions
extern int LegacyDLLExport icsneoGetRTC(void* hObject, icsSpyTime* time);
extern int LegacyDLLExport icsneoSetRTC(void* hObject, const icsSpyTime* time);

//Device Functions
extern int LegacyDLLExport icsneoGetConfiguration(void* hObject, unsigned char* pData, int* lNumBytes);
extern int LegacyDLLExport icsneoSendConfiguration(void* hObject, unsigned char* pData, int lNumBytes);
extern int LegacyDLLExport icsneoGetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoSetBitRate(void* hObject, int BitRate, int NetworkID);
extern int LegacyDLLExport icsneoSetBitRateEx(void* hObject, unsigned long BitRate, int NetworkID, int iOptions);
extern int LegacyDLLExport icsneoSetFDBitRate(void* hObject, int BitRate, int NetworkID);

extern int LegacyDLLExport icsneoGetDeviceParameters(void* hObject, char* pParameter, char* pValues, short ValuesLength);
extern int LegacyDLLExport icsneoSetDeviceParameters(void* hObject, char* pParmValue, int* pErrorIndex, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetVCAN4Settings(void* hObject, SVCAN4Settings* pSettings, int iNumBytes);
extern int LegacyDLLExport icsneoSetVCAN4Settings(void* hObject, SVCAN4Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int LegacyDLLExport icsneoGetDeviceSettingsType(void* hObject, EPlasmaIonVnetChannel_t vnetSlot, EDeviceSettingsType* pDeviceSettingsType);
extern int LegacyDLLExport icsneoSetDeviceSettings(void* hObject, SDeviceSettings* pSettings, int iNumBytes, int bSaveToEEPROM, EPlasmaIonVnetChannel_t vnetSlot);
extern int LegacyDLLExport icsneoGetDeviceSettings(void* hObject, SDeviceSettings* pSettings, int iNumBytes, EPlasmaIonVnetChannel_t vnetSlot);

extern int LegacyDLLExport icsneoLoadDefaultSettings(void* hObject);

extern int LegacyDLLExport icsneoSetUSBLatency(NeoDevice* pNeoDevice, unsigned long LatencyVal);

extern unsigned int LegacyDLLExport icsneoGetCANControllerClockFrequency(void* hObject, int lNetworkID);
extern int LegacyDLLExport icsneoSetCANParameters(void* hObject,unsigned int baudRatePrescaler,unsigned int propagationTqs,unsigned int phase1Tqs,unsigned int phase2Tqs,unsigned int syncJumpWidthTqs,unsigned int optionBits,int lNetworkID);
extern int LegacyDLLExport icsneoSetCANParametersPhilipsSJA1000(void* hObject,unsigned int btr0,unsigned int btr1,unsigned int optionBits,int lNetworkID);

extern int LegacyDLLExport icsneoGetDeviceStatus(void* hObject, icsDeviceStatus* deviceStatus, size_t* deviceStatusSize);

//Remote Device Functions
extern int LegacyDLLExport icsneoOpenRemoteNeoDevice(const char* pIPAddress,NeoDevice* pNeoDevice,void** hObject,unsigned char* bNetworkIDs,int iOptions);
extern int LegacyDLLExport icsneoFindRemoteNeoDevices(const char* pIPAddress, NeoDevice* pNeoDevice, int* pNumDevices);

//Error Functions
extern int LegacyDLLExport icsneoGetLastAPIError(void* hObject, unsigned long* pErrorNumber);
extern int LegacyDLLExport icsneoGetErrorMessages(void* hObject, int* pErrorMsgs, int* pNumberOfErrors);
extern int LegacyDLLExport icsneoGetErrorInfo(int lErrorNumber, char* szErrorDescriptionShort, char* szErrorDescriptionLong, int* lMaxLengthShort, int* lMaxLengthLong,int* lErrorSeverity,int* lRestartNeeded);

//ISO15765-2 Functions
extern int LegacyDLLExport icsneoISO15765_EnableNetworks(void* hObject, unsigned long ulNetworks);
extern int LegacyDLLExport icsneoISO15765_DisableNetworks(void* hObject);
extern int LegacyDLLExport icsneoISO15765_TransmitMessage(void* hObject, unsigned long ulNetworkID, stCM_ISO157652_TxMessage* pMsg, unsigned long ulBlockingTimeout);
extern int LegacyDLLExport icsneoISO15765_TransmitMessageEx(void* hObject,unsigned long ulNetworkID,ISO15765_2015_TxMessage* pMsg,unsigned long ulBlockingTimeout);
extern int LegacyDLLExport icsneoISO15765_ReceiveMessage(void* hObject, int ulNetworkID, stCM_ISO157652_RxMessage* pMsg);

//General Utility Functions
extern int LegacyDLLExport icsneoValidateHObject(void* hObject);

extern int LegacyDLLExport icsneoGetDLLVersion(void);
extern void LegacyDLLExport icsneoGetDLLVersionEx(unsigned long* dwMSVersion, unsigned long* dwLSVersion);

extern int LegacyDLLExport icsneoGetSerialNumber(void* hObject, unsigned int*iSerialNumber);
extern int LegacyDLLExport icsneoSerialNumberFromString(unsigned long* serial, char* data);
extern int LegacyDLLExport icsneoSerialNumberToString(unsigned long serial, char* data, unsigned long data_size);

extern int LegacyDLLExport icsneoStartSockServer(void* hObject, int iPort);
extern int LegacyDLLExport icsneoStopSockServer(void* hObject);

//CoreMini Script functions
extern int LegacyDLLExport icsneoScriptStart(void* hObject, int iLocation);
extern int LegacyDLLExport icsneoScriptStop(void* hObject);
extern int LegacyDLLExport icsneoScriptLoad(void* hObject, const unsigned char* bin, unsigned long len_bytes, int iLocation);
extern int LegacyDLLExport icsneoScriptClear(void* hObject, int iLocation);
extern int LegacyDLLExport icsneoScriptStartFBlock(void* hObject,unsigned int fb_index);
extern int LegacyDLLExport icsneoScriptGetFBlockStatus(void* hObject, unsigned int fb_index, int* piRunStatus);
extern int LegacyDLLExport icsneoScriptStopFBlock(void* hObject,unsigned int fb_index);
extern int LegacyDLLExport icsneoScriptGetScriptStatus(void* hObject, int* piStatus);
extern int LegacyDLLExport icsneoScriptReadAppSignal(void* hObject, unsigned int iIndex, double*dValue);
extern int LegacyDLLExport icsneoScriptWriteAppSignal(void* hObject, unsigned int iIndex, double dValue);

//Deprecated (but still supported in the DLL)
// icsneoOpenPortEx was marked as obsolete in icsneo40 in 2016, Please move to icsneoOpenNeoDevice
// extern int LegacyDLLExport icsneoOpenPortEx(void* lPortNumber, int lPortType, int lDriverType, int lIPAddressMSB, int lIPAddressLSBOrBaudRate, int bConfigRead, unsigned char* bNetworkID, int* hObject);
// icsneoOpenPort was marked as obsolete in icsneo40 in 2016, Please move to icsneoOpenNeoDevice
// extern int LegacyDLLExport icsneoOpenPort(int lPortNumber, int lPortType, int lDriverType, unsigned char* bNetworkID, unsigned char* bSCPIDs, int* hObject);
// icsneoFindAllCOMDevices was marked as obsolete in icsneo40 in 2016, Please move to icsneoOpenNeoDevice
// extern int LegacyDLLExport icsneoFindAllCOMDevices(int lDriverType, int lGetSerialNumbers, int lStopAtFirst, int lUSBCommOnly, int* p_lDeviceTypes, int* p_lComPorts, int* p_lSerialNumbers, int*lNumDevices);
// icsneoOpenNeoDeviceByChannels was marked as obsolete in icsneo40 in 2016, Please move to icsneoOpenNeoDevice
// extern int LegacyDLLExport icsneoOpenNeoDeviceByChannels(NeoDevice* pNeoDevice,void** hObject,unsigned char* uChannels,int iSize,int bConfigRead,int iOptions);
// extern int LegacyDLLExport icsneoLockChannels(void* hObject, unsigned char* uChannels, int iSize);
// extern int LegacyDLLExport icsneoUnlockChannels(void* hObject, unsigned char* uChannels, int iSize);
// extern int LegacyDLLExport icsneoGetChannelLockStatus(void* hObject, unsigned char* uChannels, int iSize);

//Enable Network Com Functions
extern int LegacyDLLExport icsneoEnableNetworkComEx(void* hObject, int iEnable, int iNetId);
extern int LegacyDLLExport icsneoEnableNetworkCom(void* hObject, int Enable);

//Firmware Info/Update calls
extern int LegacyDLLExport icsneoForceFirmwareUpdate(void* hObject);
extern int LegacyDLLExport icsneoGetHWFirmwareInfo(void* hObject, stAPIFirmwareInfo* pInfo);
extern int LegacyDLLExport icsneoGetDLLFirmwareInfo(void* hObject, stAPIFirmwareInfo* pInfo);
extern int LegacyDLLExport icsneoGetDLLFirmwareInfoEx(void* hObject, stAPIFirmwareInfo* pInfo, EPlasmaIonVnetChannel_t vnetSlot);
extern int LegacyDLLExport icsneoFirmwareUpdateRequired(void* hObject);

//J2534 API functions
extern int LegacyDLLExport icsneoJ2534Cmd(void* hObject, unsigned char* CmdBuf, short Len, void* pVoid);
extern int LegacyDLLExport icsneoSendRawCmds(void* hObject, icsneoVICommand* pCmdMsgs, int lNumOfCmds);
extern int LegacyDLLExport icsneoEnableBusVoltageMonitor(void* hObject, unsigned int enable, unsigned int reserved);
extern int LegacyDLLExport icsneoGetBusVoltage(void* hObject, unsigned long* pVBusVoltage, unsigned int reserved);
extern int LegacyDLLExport icsneoGetNetidforSlaveVNETs(size_t* NetworkIndex, EPlasmaIonVnetChannel_t vnetSlot);
extern int LegacyDLLExport icsneoGetVnetSimpleNetid(size_t* FullNetID);
extern int LegacyDLLExport icsneoEnableDOIPLine(void* hObject, bool bActivate);
extern int LegacyDLLExport icsneoGetMiniportAdapterInfo(void* hObject, NDIS_ADAPTER_INFORMATION* aInfo);

// Hardware Time API functions
extern int LegacyDLLExport icsneoSendHWTimeRequest(void* hObject);
extern int LegacyDLLExport icsneoReceiveHWTimeResponse(void* hObject, double* TimeHardware, unsigned long TimeOut);

//Bitsmash functions
extern int LegacyDLLExport icsneoEnableBitSmash(void* hObject,int netId,unsigned int numWaitBits,unsigned int numSmashBits,unsigned int numFrames,unsigned int timeout_ms,unsigned int optionBits,unsigned int numArbIds,unsigned int arbIds[MAX_BIT_SMASH_ARBIDS]);
extern int LegacyDLLExport icsneoDisableBitSmash(void* hObject, unsigned int reservedZero);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
