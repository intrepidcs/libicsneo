#ifndef __ICSNEOLEGACY_H_
#define __ICSNEOLEGACY_H_

#include "icsneo/platform/dynamiclib.h" // Dynamic library loading and exporting
#include "icsneo/platform/tchar.h"

#include <stdint.h>
typedef uint8_t byte; // Typedef helper for the following include
#include "icsneo/icsnVC40.h" // Definitions for structs

#ifdef __cplusplus
extern "C" {
#endif

//Basic Functions
extern int DLLExport icsneoFindNeoDevices(unsigned long DeviceTypes, NeoDevice* pNeoDevice, int* pNumDevices);
extern int DLLExport icsneoOpenNeoDevice(NeoDevice* pNeoDevice, void** hObject, unsigned char* bNetworkIDs, int bConfigRead, int bSyncToPC);
extern int DLLExport icsneoClosePort(void* hObject, int* pNumberOfErrors);
extern void DLLExport icsneoFreeObject(void* hObject);
extern int DLLExport icsneoSerialNumberToString(unsigned long serial, char* data, unsigned long data_size);

//Message Functions
extern int DLLExport icsneoGetMessages(void* hObject, icsSpyMessage* pMsg, int* pNumberOfMessages, int* pNumberOfErrors);
extern int DLLExport icsneoTxMessages(void* hObject, icsSpyMessage* pMsg, int lNetworkID, int lNumMessages);
extern int DLLExport icsneoTxMessagesEx(void* hObject,icsSpyMessage* pMsg, unsigned int lNetworkID, unsigned int lNumMessages, unsigned int* NumTxed, unsigned int zero2);
extern int DLLExport icsneoWaitForRxMessagesWithTimeOut(void* hObject, unsigned int iTimeOut);
extern int DLLExport icsneoEnableNetworkRXQueue(void* hObject, int iEnable);
extern int DLLExport icsneoGetTimeStampForMsg(void* hObject, icsSpyMessage* pMsg, double* pTimeStamp);
extern void DLLExport icsneoGetISO15765Status(void* hObject, int lNetwork, int lClearTxStatus, int lClearRxStatus, int*lTxStatus, int*lRxStatus);
extern void DLLExport icsneoSetISO15765RxParameters(void* hObject, int lNetwork, int lEnable, spyFilterLong* pFF_CFMsgFilter, icsSpyMessage* pTxMsg,
										int lCFTimeOutMs, int lFlowCBlockSize, int lUsesExtendedAddressing, int lUseHardwareIfPresent);

//Device Functions
extern int DLLExport icsneoGetConfiguration(void* hObject, unsigned char* pData, int* lNumBytes);
extern int DLLExport icsneoSendConfiguration(void* hObject, unsigned char* pData, int lNumBytes);
extern int DLLExport icsneoGetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoGetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes);
extern int DLLExport icsneoSetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes, int bSaveToEEPROM);

extern int DLLExport icsneoSetBitRate(void* hObject, int BitRate, int NetworkID);
extern int DLLExport icsneoGetDeviceParameters(void* hObject, char* pParameter, char* pValues, short ValuesLength);
extern int DLLExport icsneoSetDeviceParameters(void* hObject, char* pParmValue, int* pErrorIndex, int bSaveToEEPROM);

//Error Functions
extern int DLLExport icsneoGetLastAPIError(void* hObject, unsigned long* pErrorNumber);
extern int DLLExport icsneoGetErrorMessages(void* hObject, int* pErrorMsgs, int* pNumberOfErrors);
extern int DLLExport icsneoGetErrorInfo(int lErrorNumber, TCHAR*szErrorDescriptionShort, TCHAR*szErrorDescriptionLong, int* lMaxLengthShort, int* lMaxLengthLong,int* lErrorSeverity,int* lRestartNeeded);

//ISO15765-2 Functions
extern int DLLExport icsneoISO15765_EnableNetworks(void* hObject, unsigned long ulNetworks);
extern int DLLExport icsneoISO15765_DisableNetworks(void* hObject);
extern int DLLExport icsneoISO15765_TransmitMessage(void* hObject, unsigned long ulNetworkID, stCM_ISO157652_TxMessage* pMsg, unsigned long ulBlockingTimeout);
extern int DLLExport icsneoISO15765_ReceiveMessage(void* hObject,int ulNetworkID, stCM_ISO157652_RxMessage* pMsg);

//General Utility Functions
extern int DLLExport icsneoValidateHObject(void* hObject);
extern int DLLExport icsneoGetDLLVersion(void);
extern int DLLExport icsneoGetSerialNumber(void* hObject, unsigned int*iSerialNumber);
extern int DLLExport icsneoStartSockServer(void* hObject, int iPort);
extern int DLLExport icsneoStopSockServer(void* hObject);

//CoreMini Script functions
extern int DLLExport icsneoScriptStart(void* hObject, int iLocation);
extern int DLLExport icsneoScriptStop(void* hObject);
extern int DLLExport icsneoScriptLoad(void* hObject, const unsigned char* bin, unsigned long len_bytes, int iLocation);
extern int DLLExport icsneoScriptClear(void* hObject, int iLocation);
extern int DLLExport icsneoScriptStartFBlock(void* hObject,unsigned int fb_index);
extern int DLLExport icsneoScriptGetFBlockStatus(void* hObject, unsigned int fb_index, int* piRunStatus);
extern int DLLExport icsneoScriptStopFBlock(void* hObject,unsigned int fb_index);
extern int DLLExport icsneoScriptGetScriptStatus(void* hObject, int* piStatus);
extern int DLLExport icsneoScriptReadAppSignal(void* hObject, unsigned int iIndex, double*dValue);
extern int DLLExport icsneoScriptWriteAppSignal(void* hObject, unsigned int iIndex, double dValue);

//Deprecated (but still suppored in the DLL)
extern int DLLExport icsneoOpenPortEx(void* lPortNumber, int lPortType, int lDriverType, int lIPAddressMSB, int lIPAddressLSBOrBaudRate, int bConfigRead, unsigned char* bNetworkID, int* hObject);
extern int DLLExport icsneoOpenPort(int lPortNumber, int lPortType, int lDriverType, unsigned char* bNetworkID, unsigned char* bSCPIDs, int* hObject);
extern int DLLExport icsneoEnableNetworkCom(void* hObject, int Enable);
extern int DLLExport icsneoFindAllCOMDevices(int lDriverType, int lGetSerialNumbers, int lStopAtFirst, int lUSBCommOnly, int* p_lDeviceTypes, int* p_lComPorts, int* p_lSerialNumbers, int*lNumDevices);

#ifdef __cplusplus
} // extern "C"
#endif

#endif