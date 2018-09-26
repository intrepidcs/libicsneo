//FILE: icsneo40DLLAPI.cpp
#include "icsneo40DLLAPI.h"
#include <Tchar.h>

//Basic Functions
FINDNEODEVICES icsneoFindNeoDevices;
OPENNEODEVICE icsneoOpenNeoDevice;
CLOSEPORT icsneoClosePort;
FREEOBJECT icsneoFreeObject;
////OPENPORTEX icsneoOpenPortEx;
SERIALNUMBERTOSTRING icsneoSerialNumberToString;


//Message Functions
GETMESSAGES icsneoGetMessages;
TXMESSAGES icsneoTxMessages;
TXMESSAGESEX icsneoTxMessagesEx;
WAITFORRXMSGS icsneoWaitForRxMessagesWithTimeOut;
GETTSFORMSG icsneoGetTimeStampForMsg;
ENABLERXQUEUE icsneoEnableNetworkRXQueue;
GETISO15765STATUS icsneoGetISO15765Status;
SETISO15765RXPARMS icsneoSetISO15765RxParameters;
////ENABLENETWORKCOM icsneoEnableNetworkCom;


//Device Functions
GETCONFIG icsneoGetConfiguration;
SENDCONFIG icsneoSendConfiguration;
GETFIRESETTINGS icsneoGetFireSettings;
SETFIRESETTINGS icsneoSetFireSettings;
GETVCAN3SETTINGS icsneoGetVCAN3Settings;
SETVCAN3SETTINGS icsneoSetVCAN3Settings;

GETFIRE2SETTINGS icsneoGetFire2Settings;
SETFIRE2SETTINGS icsneoSetFire2Settings;
GETVCANRFSETTINGS icsneoGetVCANRFSettings;
SETVCANRFSETTINGS icsneoSetVCANRFSettings;
GETRADGalaxySETTINGS icsneoGetRADGalaxySettings;
SETRADGalaxySETTINGS icsneoSetRADGalaxySettings;
GETRADSTAR2SETTINGS icsneoGetRADStar2Settings;
SETRADSTAR2SETTINGS icsneoSetRADStar2Settings;
GETVCAN412SETTINGS icsneoGetVCAN412Settings;
SETVCAN412SETTINGS icsneoSetVCAN412Settings;

SETBITRATE icsneoSetBitRate;
GETDEVICEPARMS icsneoGetDeviceParameters;
SETDEVICEPARMS icsneoSetDeviceParameters;

//Error Functions
GETLASTAPIERROR icsneoGetLastAPIError;
GETERRMSGS icsneoGetErrorMessages;
GETERRORINFO icsneoGetErrorInfo;

//ISO15765-2
ISO15765ENABLENETWORKS icsneoISO15765_EnableNetworks;
ISO15765DISABLENETWORKS icsneoISO15765_DisableNetworks;
ISO15765TXMSG icsneoISO15765_TransmitMessage;
ISO15765RXMSG icsneoISO15765_ReceiveMessage;

//General Utility Functions
VALIDATEHOBJECT icsneoValidateHObject;
GETDLLVERSION icsneoGetDLLVersion;
GETSERIALNUMBER icsneoGetSerialNumber;
STARTSOCKSERVER icsneoStartSockServer;
STOPSOCKSERVER icsneoStopSockServer;

//CoreMini Functions
SCRIPTLOAD icsneoScriptLoad;
SCRIPTSTART icsneoScriptStart;
SCRIPTSTOP icsneoScriptStop;
SCRIPTCLEAR icsneoScriptClear;
SCRIPTSTARTFBLOCK icsneoScriptStartFBlock;
SCRIPTSTOPFBLOCK icsneoScriptStopFBlock;
SCRIPTGETFBLOCKSTATUS icsneoScriptGetFBlockStatus;
SCRIPTGETSCRIPTSTATUS icsneoScriptGetScriptStatus;
SCRIPTREADAPPSIGNAL icsneoScriptReadAppSignal;
SCRIPTWRITEAPPSIGNAL icsneoScriptWriteAppSignal;
//SCRIPTREADRXMESSAGE icsneoScriptReadRxMessage;
//SCRIPTREADTXMESSAGE icsneoScriptReadTxMessage;
//SCRIPTWRITERXMESSAGE icsneoScriptWriteRxMessage;
//SCRIPTWRITETXMESSAGE icsneoScriptWriteTxMessage;


//The following are valid strings for setting parameters on devices 
//using the icsneoGetDeviceParameters() and icsneoSetDeviceParameters() functions
char *FireParameters[] =
{
	"can1", "can2", "can3", "can4", "swcan", "lsftcan", "lin1", "lin2",
	"lin3", "lin4", "cgi_baud", "cgi_tx_ifs_bit_times",
	"cgi_rx_ifs_bit_times", "cgi_chksum_enable", "network_enables", 
	"network_enabled_on_boot", "pwm_man_timeout", "pwr_man_enable", 
	"misc_io_initial_ddr", "misc_io_initial_latch", "misc_io_analog_enable", 
	"misc_io_report_period", "misc_io_on_report_events", "ain_sample_period", 
	"ain_threshold", "iso15765_separation_time_offset", "iso9141_kwp_settings", 
	"perf_en", "iso_parity", "iso_msg_termination", "network_enables_2"
};


char *VCAN3Parameters[] =
{
	"can1", "can2", "network_enables", "network_enabled_on_boot", "iso15765_separation_time_offset",
	"perf_en", "misc_io_initial_ddr", "misc_io_initial_latch", "misc_io_report_period", 
	"misc_io_on_report_events"
};

char *CANParameters[] = 
{
     "Mode", "SetBaudrate", "Baudrate", "NetworkType", "TqSeg1",
	 "TqSeg2", "TqProp", "TqSync", "BRP", "auto_baud"
};

char *SWCANParameters[] = 
{
	 "Mode", "SetBaudrate", "Baudrate", "NetworkType", "TqSeg1", "TqSeg2", 
	 "TqProp", "TqSync", "BRP", "high_speed_auto_switch", "auto_baud"
};

char *LINParameters[] = 
{
	 "Baudrate", "spbrg", "brgh", "MasterResistor", "Mode"
};

char *ISOKWPParms[] =
{
	 "Baudrate", "spbrg", "brgh", "init_steps", "init_step_count", 
	 "p2_500us", "p3_500us", "p4_500us", "chksum_enabled"
};


bool LoadDLLAPI(HINSTANCE &hAPIDLL)
{
 	if((hAPIDLL = LoadLibrary(_T("icsneo40.dll"))) == NULL)
		return false;



	icsneoFindNeoDevices =    (FINDNEODEVICES) GetProcAddress(hAPIDLL,              "icsneoFindNeoDevices");
	icsneoOpenNeoDevice =     (OPENNEODEVICE) GetProcAddress(hAPIDLL,               "icsneoOpenNeoDevice");
	icsneoClosePort =         (CLOSEPORT) GetProcAddress(hAPIDLL,                   "icsneoClosePort");	
	icsneoFreeObject =        (FREEOBJECT) GetProcAddress(hAPIDLL,                  "icsneoFreeObject");
////    icsneoOpenPortEx =        (OPENPORTEX) GetProcAddress(hAPIDLL,                  "icsneoOpenPortEx");
////	icsneoEnableNetworkCom = (ENABLENETWORKCOM) GetProcAddress(hAPIDLL,             "icsneoEnableNetworkCom");
	icsneoGetDLLVersion = (GETDLLVERSION) GetProcAddress(hAPIDLL,					"icsneoGetDLLVersion");
	icsneoSerialNumberToString  = (SERIALNUMBERTOSTRING) GetProcAddress(hAPIDLL, "icsneoSerialNumberToString");

    icsneoTxMessages =        (TXMESSAGES) GetProcAddress(hAPIDLL,                  "icsneoTxMessages");
	icsneoTxMessagesEx =	  (TXMESSAGESEX) GetProcAddress(hAPIDLL,				"icsneoTxMessagesEx");
	icsneoGetMessages =       (GETMESSAGES) GetProcAddress(hAPIDLL,                 "icsneoGetMessages");
	icsneoWaitForRxMessagesWithTimeOut = (WAITFORRXMSGS) GetProcAddress(hAPIDLL,    "icsneoWaitForRxMessagesWithTimeOut");
    icsneoGetTimeStampForMsg = (GETTSFORMSG) GetProcAddress(hAPIDLL,                "icsneoGetTimeStampForMsg");
    icsneoEnableNetworkRXQueue = (ENABLERXQUEUE) GetProcAddress(hAPIDLL,            "icsneoEnableNetworkRXQueue");
    icsneoGetISO15765Status =  (GETISO15765STATUS) GetProcAddress(hAPIDLL,          "icsneoGetISO15765Status");
    icsneoSetISO15765RxParameters = (SETISO15765RXPARMS) GetProcAddress(hAPIDLL,    "icsneoSetISO15765RxParameters");

	icsneoISO15765_EnableNetworks = (ISO15765ENABLENETWORKS) GetProcAddress(hAPIDLL, "icsneoISO15765_EnableNetworks");
	icsneoISO15765_DisableNetworks = (ISO15765DISABLENETWORKS) GetProcAddress(hAPIDLL, "icsneoISO15765_DisableNetworks");
	icsneoISO15765_TransmitMessage = (ISO15765TXMSG)GetProcAddress(hAPIDLL, "icsneoISO15765_TransmitMessage");
	icsneoISO15765_ReceiveMessage = (ISO15765RXMSG)GetProcAddress(hAPIDLL, "icsneoISO15765_ReceiveMessage");

	icsneoGetConfiguration =  (GETCONFIG) GetProcAddress(hAPIDLL,                   "icsneoGetConfiguration");
  	icsneoSendConfiguration = (SENDCONFIG) GetProcAddress(hAPIDLL,                  "icsneoSendConfiguration");
    icsneoGetFireSettings =   (GETFIRESETTINGS) GetProcAddress(hAPIDLL,             "icsneoGetFireSettings");
	icsneoSetFireSettings =   (SETFIRESETTINGS) GetProcAddress(hAPIDLL,             "icsneoSetFireSettings");
	icsneoGetVCAN3Settings =  (GETVCAN3SETTINGS) GetProcAddress(hAPIDLL,            "icsneoGetVCAN3Settings");
    icsneoSetVCAN3Settings =  (SETVCAN3SETTINGS) GetProcAddress(hAPIDLL,            "icsneoSetVCAN3Settings");


	icsneoGetVCANRFSettings =  (GETVCANRFSETTINGS) GetProcAddress(hAPIDLL,            "icsneoGetVCANRFSettings");
    icsneoSetVCANRFSettings =  (SETVCANRFSETTINGS) GetProcAddress(hAPIDLL,            "icsneoSetVCANRFSettings");
	
	icsneoGetFire2Settings =   (GETFIRE2SETTINGS) GetProcAddress(hAPIDLL,             "icsneoGetFire2Settings");
	icsneoSetFire2Settings =   (SETFIRE2SETTINGS) GetProcAddress(hAPIDLL,             "icsneoSetFire2Settings");
	
	icsneoGetRADGalaxySettings =   (GETRADGalaxySETTINGS) GetProcAddress(hAPIDLL,             "icsneoGetRADGalaxySettings");
	icsneoSetRADGalaxySettings =   (SETRADGalaxySETTINGS) GetProcAddress(hAPIDLL,             "icsneoSetRADGalaxySettings");

	icsneoGetRADStar2Settings =   (GETRADSTAR2SETTINGS) GetProcAddress(hAPIDLL,             "icsneoGetRADStar2Settings");
	icsneoSetRADStar2Settings =   (SETRADSTAR2SETTINGS) GetProcAddress(hAPIDLL,             "icsneoSetRADStar2Settings");
	
	icsneoGetVCAN412Settings =  (GETVCAN412SETTINGS) GetProcAddress(hAPIDLL,            "icsneoGetVCAN412Settings");
    icsneoSetVCAN412Settings =  (SETVCAN412SETTINGS) GetProcAddress(hAPIDLL,            "icsneoSetVCAN412Settings");



	icsneoSetBitRate =        (SETBITRATE)       GetProcAddress(hAPIDLL,            "icsneoSetBitRate");
	icsneoGetDeviceParameters = (GETDEVICEPARMS) GetProcAddress(hAPIDLL,            "icsneoGetDeviceParameters");
	icsneoSetDeviceParameters = (SETDEVICEPARMS) GetProcAddress(hAPIDLL,            "icsneoSetDeviceParameters");

	icsneoGetLastAPIError =   (GETLASTAPIERROR) GetProcAddress(hAPIDLL,             "icsneoGetLastAPIError");
    icsneoGetErrorMessages = (GETERRMSGS) GetProcAddress(hAPIDLL,                   "icsneoGetErrorMessages");
    icsneoGetErrorInfo =     (GETERRORINFO) GetProcAddress(hAPIDLL,                 "icsneoGetErrorInfo");

	icsneoScriptLoad =        (SCRIPTLOAD) GetProcAddress(hAPIDLL,                  "icsneoScriptLoad");
	icsneoScriptStart =       (SCRIPTSTART) GetProcAddress(hAPIDLL,                 "icsneoScriptStart");
	icsneoScriptStop =        (SCRIPTSTOP) GetProcAddress(hAPIDLL,                  "icsneoScriptStop");
	icsneoScriptClear =       (SCRIPTCLEAR) GetProcAddress(hAPIDLL,                 "icsneoScriptClear");
	icsneoScriptStartFBlock = (SCRIPTSTARTFBLOCK) GetProcAddress(hAPIDLL,           "icsneoScriptStartFBlock");
	icsneoScriptStopFBlock =  (SCRIPTSTOPFBLOCK) GetProcAddress(hAPIDLL,            "icsneoScriptStopFBlock");
	icsneoScriptGetFBlockStatus = (SCRIPTGETFBLOCKSTATUS) GetProcAddress(hAPIDLL,   "icsneoScriptGetFBlockStatus");
	icsneoScriptGetScriptStatus = (SCRIPTGETSCRIPTSTATUS) GetProcAddress(hAPIDLL, "icsneoScriptGetScriptStatus");
	icsneoScriptReadAppSignal = (SCRIPTREADAPPSIGNAL) GetProcAddress(hAPIDLL,       "icsneoScriptReadAppSignal");
    icsneoScriptWriteAppSignal = (SCRIPTWRITEAPPSIGNAL) GetProcAddress(hAPIDLL,     "icsneoScriptWriteAppSignal");

	

    if(!icsneoFindNeoDevices || !icsneoOpenNeoDevice || !icsneoClosePort || !icsneoFreeObject ||  
	   !icsneoTxMessages || !icsneoGetMessages || !icsneoWaitForRxMessagesWithTimeOut ||
       !icsneoGetTimeStampForMsg || !icsneoEnableNetworkRXQueue || !icsneoGetISO15765Status || !icsneoTxMessagesEx ||
       !icsneoSetISO15765RxParameters || !icsneoGetConfiguration || !icsneoSendConfiguration ||
       !icsneoGetFireSettings || !icsneoSetFireSettings || !icsneoGetVCAN3Settings ||
       !icsneoSetVCAN3Settings || !icsneoGetVCANRFSettings || !icsneoSetVCANRFSettings || !icsneoGetFire2Settings ||
	   !icsneoGetVCAN412Settings || !icsneoSetVCAN412Settings ||
	   !icsneoSetFire2Settings || !icsneoGetRADGalaxySettings || !icsneoSetRADGalaxySettings || 
	   !icsneoSetBitRate || !icsneoGetDeviceParameters || !icsneoSerialNumberToString ||
       !icsneoSetDeviceParameters || !icsneoGetLastAPIError || !icsneoGetErrorMessages ||
       !icsneoGetErrorInfo || !icsneoScriptLoad || !icsneoScriptStart || !icsneoScriptStop ||
       !icsneoScriptClear || !icsneoScriptStartFBlock || !icsneoScriptStopFBlock ||
       !icsneoScriptGetFBlockStatus || !icsneoScriptGetScriptStatus || !icsneoScriptReadAppSignal ||
       !icsneoScriptWriteAppSignal || !icsneoGetDLLVersion)
	{
		FreeLibrary(hAPIDLL);
		return false;
	}

   return true;
}	


void UnloadDLLAPI(HINSTANCE &hAPIDLL)
{
	if(hAPIDLL)
		FreeLibrary(hAPIDLL);
}