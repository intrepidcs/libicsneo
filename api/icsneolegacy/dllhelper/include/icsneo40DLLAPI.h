//FILE: icsneo40DLLAPI.H

#include <windows.h>
#include "icsnVC40.h"



bool LoadDLLAPI(HINSTANCE &hAPIDLL);
void UnloadDLLAPI(HINSTANCE &hAPIDLL);

//Basic Functions
typedef int  (__stdcall *FINDNEODEVICES)(unsigned long DeviceTypes, NeoDevice *pNeoDevice, int *pNumDevices);
typedef int  (__stdcall *OPENNEODEVICE)(NeoDevice *pNeoDevice, void * hObject, unsigned char *bNetworkIDs, int bConfigRead, int bSyncToPC);
typedef int  (__stdcall *CLOSEPORT)(void * hObject, int *pNumberOfErrors); 
typedef void (__stdcall *FREEOBJECT)(void * hObject);
typedef int  (__stdcall *OPENPORTEX)(void * lPortNumber, int lPortType, int lDriverType, int lIPAddressMSB, int lIPAddressLSBOrBaudRate, 
							         int bConfigRead, unsigned char *bNetworkID, int * hObject);
typedef int  (__stdcall *SERIALNUMBERTOSTRING) (unsigned long serial, char *data,unsigned long data_size);

//Message Functions
typedef int  (__stdcall *GETMESSAGES)(void * hObject, icsSpyMessage *pMsg, int * pNumberOfMessages, int * pNumberOfErrors); 
typedef int  (__stdcall *TXMESSAGES)(void * hObject, icsSpyMessage *pMsg, int lNetworkID, int lNumMessages); 
typedef int  (__stdcall *TXMESSAGESEX)(void * hObject,icsSpyMessage *pMsg, unsigned int lNetworkID,unsigned int lNumMessages,unsigned int *NumTxed, unsigned int zero2);
typedef int  (__stdcall *WAITFORRXMSGS)(void * hObject, unsigned int iTimeOut);
typedef int  (__stdcall *ENABLERXQUEUE)(void * hObject, int iEnable);
typedef int  (__stdcall *GETTSFORMSG)(void * hObject, icsSpyMessage *pMsg, double *pTimeStamp);
typedef void (__stdcall *GETISO15765STATUS)(void * hObject, int lNetwork, int lClearTxStatus, 
											int lClearRxStatus, int *lTxStatus, int *lRxStatus);
typedef void (__stdcall *SETISO15765RXPARMS)(void * hObject, int lNetwork, int lEnable, 
			                                spyFilterLong *pFF_CFMsgFilter, icsSpyMessage *pTxMsg, 
											int lCFTimeOutMs, int lFlowCBlockSize,
											int lUsesExtendedAddressing, int lUseHardwareIfPresent);
typedef int (__stdcall *DOWNLOADISO15765_2_TXSCRIPT)(void * hObject, unsigned int NetworkID);
typedef int (__stdcall *CLEARISO15765_2_TXSCRIPT)(void * hObject);
typedef int (__stdcall *TXISO15765_2_MESSAGE)(void * hObject, stCM_ISO157652_TxMessage *pTxMessage);

//Device Functions
typedef int (__stdcall *GETCONFIG)(void * hObject, unsigned char * pData, int * lNumBytes);
typedef int (__stdcall *SENDCONFIG)(void * hObject, unsigned char * pData, int lNumBytes); 
typedef int (__stdcall *GETFIRESETTINGS)(void * hObject, SFireSettings *pSettings, int iNumBytes);
typedef int (__stdcall *SETFIRESETTINGS)(void * hObject, SFireSettings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETVCAN3SETTINGS)(void * hObject, SVCAN3Settings *pSettings, int iNumBytes);
typedef int (__stdcall *SETVCAN3SETTINGS)(void * hObject, SVCAN3Settings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETFIRE2SETTINGS)(void * hObject, SFire2Settings *pSettings, int iNumBytes);
typedef int (__stdcall *SETFIRE2SETTINGS)(void * hObject, SFire2Settings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETVCANRFSETTINGS)(void * hObject, SVCANRFSettings *pSettings, int iNumBytes);
typedef int (__stdcall *SETVCANRFSETTINGS)(void * hObject, SVCANRFSettings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETVCAN412SETTINGS)(void * hObject, SVCAN412Settings *pSettings, int iNumBytes);
typedef int (__stdcall *SETVCAN412SETTINGS)(void * hObject, SVCAN412Settings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETRADGalaxySETTINGS)(void * hObject, SRADGalaxySettings *pSettings, int iNumBytes);
typedef int (__stdcall *SETRADGalaxySETTINGS)(void * hObject, SRADGalaxySettings *pSettings, int iNumBytes, int bSaveToEEPROM);

typedef int (__stdcall *GETRADSTAR2SETTINGS)(void * hObject, SRADStar2Settings *pSettings, int iNumBytes);
typedef int (__stdcall *SETRADSTAR2SETTINGS)(void * hObject, SRADStar2Settings *pSettings, int iNumBytes, int bSaveToEEPROM);


typedef int (__stdcall *SETBITRATE)(void * hObject, int BitRate, int NetworkID);
typedef int (__stdcall *GETDEVICEPARMS)(void * hObject, char *pParameter, char *pValues, short ValuesLength);
typedef int (__stdcall *SETDEVICEPARMS)(void * hObject, char *pParmValue, int *pErrorIndex, int bSaveToEEPROM);

//Error Functions
typedef int (__stdcall *GETLASTAPIERROR)(void * hObject, unsigned long *pErrorNumber);
typedef int (__stdcall *GETERRMSGS)(void * hObject, int * pErrorMsgs, int * pNumberOfErrors);
typedef int (__stdcall *GETERRORINFO)(int lErrorNumber, TCHAR *szErrorDescriptionShort, 
										TCHAR *szErrorDescriptionLong, int * lMaxLengthShort,
                                        int * lMaxLengthLong,int * lErrorSeverity,int * lRestartNeeded);

//ISO15765-2 Functions
typedef int (__stdcall *ISO15765ENABLENETWORKS)(void * hObject, unsigned long ulNetworks);
typedef int (__stdcall *ISO15765DISABLENETWORKS)(void * hObject);
typedef int (__stdcall *ISO15765TXMSG)(void * hObject, unsigned long ulNetworkID,stCM_ISO157652_TxMessage *pMsg, unsigned long ulBlockingTimeout);
typedef int (__stdcall *ISO15765RXMSG)(void * hObject,int ulNetworkID,stCM_ISO157652_RxMessage * pMsg);

//General Utility Functions
typedef int (__stdcall *VALIDATEHOBJECT)(void * hObject);
typedef int (__stdcall *GETDLLVERSION)(void);
typedef int (__stdcall *GETSERIALNUMBER)(void * hObject, unsigned int *iSerialNumber);
typedef int (__stdcall *STARTSOCKSERVER)(void * hObject, int iPort);
typedef int (__stdcall *STOPSOCKSERVER)(void * hObject);

//CoreMini Script functions
typedef int  (__stdcall *SCRIPTSTART)(void * hObject, int iLocation);  
typedef int  (__stdcall *SCRIPTSTOP)(void * hObject);  
typedef int  (__stdcall *SCRIPTLOAD)(void * hObject, const unsigned char * bin, unsigned long len_bytes, int iLocation);
typedef int  (__stdcall *SCRIPTCLEAR)(void * hObject, int iLocation);
typedef int  (__stdcall *SCRIPTSTARTFBLOCK)(void * hObject,unsigned int fb_index);
typedef int  (__stdcall *SCRIPTGETFBLOCKSTATUS)(void * hObject, unsigned int fb_index, int *piRunStatus);
typedef int  (__stdcall *SCRIPTSTOPFBLOCK)(void * hObject,unsigned int fb_index);
typedef int  (__stdcall *SCRIPTGETSCRIPTSTATUS)(void * hObject, int *piStatus);
typedef int  (__stdcall *SCRIPTREADAPPSIGNAL)(void * hObject, unsigned int iIndex, double *dValue);
typedef int  (__stdcall *SCRIPTWRITEAPPSIGNAL)(void * hObject, unsigned int iIndex, double dValue);
typedef int  (__stdcall *SCRIPTREADISO15765TXMESSAGE)(void * hObject, unsigned int iIndex, stCM_ISO157652_TxMessage *pTxMessage);
typedef int  (__stdcall *SCRIPTWRITEISO15765TXMESSAGE)(void * hObject, unsigned int iIndex, const stCM_ISO157652_TxMessage *pTxMessage);


//Deprecated (but still suppored in the DLL)
//////typedef int  (__stdcall *OPENPORTEX)(int lPortSerialNumber, int lPortType, int lDriverType, 
//////					                 int lIPAddressMSB, int lIPAddressLSBOrBaudRate,int bConfigRead, 
//////				                     unsigned char * bNetworkID, int * hObject);
//////
//////typedef int  (__stdcall *OPENPORT)(int lPortNumber, int lPortType, int lDriverType, 
//////					               unsigned char *bNetworkID, unsigned char *bSCPIDs,  int * hObject);
//////typedef int (__stdcall *ENABLENETWORKCOM)(void * hObject, int Enable);
//////typedef int (__stdcall *FINDCOMDEVICES)(int lDriverType,  int lGetSerialNumbers, int lStopAtFirst, int lUSBCommOnly,
//////							            int *p_lDeviceTypes, int *p_lComPorts, int *p_lSerialNumbers, int *lNumDevices); 



//Basic Functions
extern FINDNEODEVICES icsneoFindNeoDevices;
extern OPENNEODEVICE icsneoOpenNeoDevice;
extern CLOSEPORT icsneoClosePort;
extern FREEOBJECT icsneoFreeObject;
extern SERIALNUMBERTOSTRING icsneoSerialNumberToString;
////extern OPENPORTEX icsneoOpenPortEx;
////extern ENABLENETWORKCOM icsneoEnableNetworkCom;

//Message Functions
extern GETMESSAGES icsneoGetMessages;
extern TXMESSAGES icsneoTxMessages;
extern TXMESSAGESEX icsneoTxMessagesEx;
extern WAITFORRXMSGS icsneoWaitForRxMessagesWithTimeOut;
extern GETTSFORMSG icsneoGetTimeStampForMsg;
extern ENABLERXQUEUE icsneoEnableNetworkRXQueue;
extern GETISO15765STATUS icsneoGetISO15765Status;
extern SETISO15765RXPARMS icsneoSetISO15765RxParameters;
extern DOWNLOADISO15765_2_TXSCRIPT icsneoDownloadISO15765_2_TXScript;
extern CLEARISO15765_2_TXSCRIPT icsneoClearISO15765_2_TxScript;
extern TXISO15765_2_MESSAGE icsneoTransmitISO15765_2_Message;


//Device Functions
extern GETCONFIG icsneoGetConfiguration;
extern SENDCONFIG icsneoSendConfiguration;
extern GETFIRESETTINGS icsneoGetFireSettings;
extern SETFIRESETTINGS icsneoSetFireSettings;
extern GETVCAN3SETTINGS icsneoGetVCAN3Settings;
extern SETVCAN3SETTINGS icsneoSetVCAN3Settings;


extern GETFIRE2SETTINGS icsneoGetFire2Settings;
extern SETFIRE2SETTINGS icsneoSetFire2Settings;
extern GETVCANRFSETTINGS icsneoGetVCANRFSettings;
extern SETVCANRFSETTINGS icsneoSetVCANRFSettings;
extern GETRADGalaxySETTINGS icsneoGetRADGalaxySettings;
extern SETRADGalaxySETTINGS icsneoSetRADGalaxySettings;
extern GETRADSTAR2SETTINGS icsneoGetRADStar2Settings;
extern SETRADSTAR2SETTINGS icsneoSetRADStar2Settings;
extern GETVCAN412SETTINGS icsneoGetVCAN412Settings;
extern SETVCAN412SETTINGS icsneoSetVCAN412Settings;

extern SETBITRATE icsneoSetBitRate;
extern GETDEVICEPARMS icsneoGetDeviceParameters;
extern SETDEVICEPARMS icsneoSetDeviceParameters;

//Error Functions
extern GETLASTAPIERROR icsneoGetLastAPIError;
extern GETERRMSGS icsneoGetErrorMessages;
extern GETERRORINFO icsneoGetErrorInfo;

//ISO15765-2
extern ISO15765ENABLENETWORKS icsneoISO15765_EnableNetworks;
extern ISO15765DISABLENETWORKS icsneoISO15765_DisableNetworks;
extern ISO15765TXMSG icsneoISO15765_TransmitMessage;
extern ISO15765RXMSG icsneoISO15765_ReceiveMessage;

//General Utility Functions
extern VALIDATEHOBJECT icsneoValidateHObject;
extern GETDLLVERSION icsneoGetDLLVersion;
extern GETSERIALNUMBER icsneoGetSerialNumber;
extern STARTSOCKSERVER icsneoStartSockServer;
extern STOPSOCKSERVER icsneoStopSockServer;

//CoreMini Functions
extern SCRIPTSTART icsneoScriptStart;
extern SCRIPTSTOP icsneoScriptStop;
extern SCRIPTLOAD icsneoScriptLoad;
extern SCRIPTCLEAR icsneoScriptClear;
extern SCRIPTSTARTFBLOCK icsneoScriptStartFBlock;
extern SCRIPTGETFBLOCKSTATUS icsneoScriptGetFBlockStatus;
extern SCRIPTSTOPFBLOCK icsneoScriptStopFBlock;
extern SCRIPTGETSCRIPTSTATUS icsneoScriptGetScriptStatus;
extern SCRIPTREADAPPSIGNAL icsneoScriptReadAppSignal;
extern SCRIPTWRITEAPPSIGNAL icsneoScriptWriteAppSignal;
//extern SCRIPTREADRXMESSAGE icsneoScriptReadRxMessage;
//extern SCRIPTREADTXMESSAGE icsneoScriptReadTxMessage;
//extern SCRIPTWRITERXMESSAGE icsneoScriptWriteRxMessage;
//extern SCRIPTWRITETXMESSAGE icsneoScriptWriteTxMessage;
extern SCRIPTREADISO15765TXMESSAGE icsneoScriptReadISO15765_2_TxMessage;
extern SCRIPTWRITEISO15765TXMESSAGE icsneoScriptWriteISO15765_2_TxMessage;






