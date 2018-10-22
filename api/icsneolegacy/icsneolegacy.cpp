#ifndef __cplusplus
#error "icsneoc.cpp must be compiled with a C++ compiler!"
#endif

#define ICSNEOC_MAKEDLL
#include "icsneo/icsneolegacy.h"

#undef ICSNEOC_MAKEDLL
#include "icsneo/icsneoc.h"

#include "icsneo/communication/network.h"
#include <map>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(disable : 4100) // unreferenced formal parameter
#endif


using namespace icsneo;

typedef uint64_t legacymaphandle_t;
static std::map<legacymaphandle_t, neodevice_t> neodevices;

static NeoDevice OldNeoDeviceFromNew(const neodevice_t* newnd) {
	NeoDevice oldnd = { 0 };
	oldnd.DeviceType = newnd->type;
	oldnd.SerialNumber = icsneo_serialStringToNum(newnd->serial);
	oldnd.NumberOfClients = 0;
	oldnd.MaxAllowedClients = 1;
	static_assert(sizeof(neodevice_handle_t) == sizeof(oldnd.Handle), "neodevice_handle_t size must be sizeof(int) for compatibility reasons");
	oldnd.Handle = newnd->handle;
	return oldnd;
}

//Basic Functions
int icsneoFindNeoDevices(unsigned long DeviceTypes, NeoDevice* pNeoDevice, int* pNumDevices) {
	constexpr size_t MAX_DEVICES = 255;
	size_t count = MAX_DEVICES;

	if(pNumDevices == nullptr)
		return 0;

	if(pNeoDevice == nullptr) {
		icsneo_findAllDevices(nullptr, &count);
		*pNumDevices = (int)count;
		return 1;
	}

	size_t bufferSize = (size_t)*pNumDevices;
	if(*pNumDevices < 0 || bufferSize > MAX_DEVICES)
		return 0;

	neodevice_t devices[MAX_DEVICES];
	icsneo_findAllDevices(devices, &count);
	if(bufferSize < count)
		count = bufferSize;
	*pNumDevices = (int)count;

	for(size_t i = 0; i < count; i++) {
		pNeoDevice[i] = OldNeoDeviceFromNew(&devices[i]); // Write out into user memory
		neodevices[uint64_t(devices[i].handle) << 32 | icsneo_serialStringToNum(devices[i].serial)] = devices[i]; // Fill the look up table
	}
	
	return 1;
}

int icsneoOpenNeoDevice(NeoDevice* pNeoDevice, void** hObject, unsigned char* bNetworkIDs, int bConfigRead, int bSyncToPC) {
	if(pNeoDevice == nullptr || hObject == nullptr)
		return false;

	neodevice_t* device;
	try {
		device = &neodevices.at(uint64_t(pNeoDevice->Handle) << 32 | pNeoDevice->SerialNumber);
	} catch(std::out_of_range& e) {
		return false;
	}

	*hObject = device;
	if(!icsneo_openDevice(device))
		return false;
	icsneo_setPollingMessageLimit(device, 20000);
	icsneo_enableMessagePolling(device);
	return icsneo_goOnline(device);
}

int icsneoClosePort(void* hObject, int* pNumberOfErrors) {
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;

	return icsneo_closeDevice(device);
}

// Memory is now managed automatically, this function is unneeded
void icsneoFreeObject(void* hObject) { (void)hObject; return; }

int icsneoSerialNumberToString(unsigned long serial, char* data, unsigned long data_size) {
	size_t length = (size_t)data_size;
	return icsneo_serialNumToString((uint32_t)serial, data, &length);
}

//Message Functions
int icsneoGetMessages(void* hObject, icsSpyMessage* pMsg, int* pNumberOfMessages, int* pNumberOfErrors) {
	static neomessage_t messages[20000];
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;

	size_t messageCount = 20000;
	if(!icsneo_getMessages(device, messages, &messageCount))
		return false;
	
	*pNumberOfMessages = (int)messageCount;
	*pNumberOfErrors = 0;

	memset(pMsg, 0, sizeof(icsSpyMessage) * messageCount);
	for(size_t i = 0; i < messageCount; i++) {
		icsSpyMessage& oldmsg = pMsg[i];
		neomessage_t& newmsg = messages[i];
		oldmsg.NumberBytesData = (uint8_t)std::min(newmsg.length, (size_t)255);
		oldmsg.NumberBytesHeader = 4;
		oldmsg.ExtraDataPtr = (void*)newmsg.data;
		memcpy(oldmsg.Data, newmsg.data, std::min(newmsg.length, (size_t)8));
		oldmsg.ArbIDOrHeader = *(uint32_t*)newmsg.header;
		oldmsg.ExtraDataPtrEnabled = newmsg.length > 8;
		oldmsg.NetworkID = newmsg.netid;
		switch(Network::Type(newmsg.type)) {
			case Network::Type::CAN:
				oldmsg.Protocol = SPY_PROTOCOL_CAN;
				break;
			// TODO Handle this better
		}
	}

	return true;
}

int icsneoTxMessages(void* hObject, icsSpyMessage* pMsg, int lNetworkID, int lNumMessages) {
	// TODO Implement
	return false;
}

int icsneoTxMessagesEx(void* hObject, icsSpyMessage* pMsg, unsigned int lNetworkID, unsigned int lNumMessages, unsigned int* NumTxed, unsigned int zero2) {
	// TODO Implement
	return false;
}

int icsneoWaitForRxMessagesWithTimeOut(void* hObject, unsigned int iTimeOut) {
	// TODO Implement
	return false;
}

int icsneoEnableNetworkRXQueue(void* hObject, int iEnable) {
	// TODO Implement
	return false;
}

int icsneoGetTimeStampForMsg(void* hObject, icsSpyMessage* pMsg, double* pTimeStamp) {
	// TODO Implement
	return false;
}

void icsneoGetISO15765Status(void* hObject, int lNetwork, int lClearTxStatus, int lClearRxStatus, int*lTxStatus, int*lRxStatus) {
	// TODO Implement
	return;
}

void icsneoSetISO15765RxParameters(void* hObject, int lNetwork, int lEnable, spyFilterLong* pFF_CFMsgFilter, icsSpyMessage* pTxMsg,
	int lCFTimeOutMs, int lFlowCBlockSize, int lUsesExtendedAddressing, int lUseHardwareIfPresent) {
	// TODO Implement
	return;
}

//Device Functions
int icsneoGetConfiguration(void* hObject, unsigned char* pData, int* lNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSendConfiguration(void* hObject, unsigned char* pData, int lNumBytes) {
	// TODO Implement
	return false;
}

int icsneoGetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoGetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes) {
	// TODO Implement
	return false;
}

int icsneoSetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

int icsneoSetBitRate(void* hObject, int BitRate, int NetworkID) {
	// TODO Implement
	return false;
}

int icsneoGetDeviceParameters(void* hObject, char* pParameter, char* pValues, short ValuesLength) {
	// TODO Implement
	return false;
}

int icsneoSetDeviceParameters(void* hObject, char* pParmValue, int* pErrorIndex, int bSaveToEEPROM) {
	// TODO Implement
	return false;
}

//Error Functions
int icsneoGetLastAPIError(void* hObject, unsigned long* pErrorNumber) {
	// TODO Implement
	return false;
}

int icsneoGetErrorMessages(void* hObject, int* pErrorMsgs, int* pNumberOfErrors) {
	// TODO Implement
	return false;
}

int icsneoGetErrorInfo(int lErrorNumber, TCHAR* szErrorDescriptionShort, TCHAR* szErrorDescriptionLong, int* lMaxLengthShort, int* lMaxLengthLong, int* lErrorSeverity, int* lRestartNeeded) {
	// TODO Implement
	return false;
}

//ISO15765-2 Functions
int icsneoISO15765_EnableNetworks(void* hObject, unsigned long ulNetworks) {
	// TODO Implement
	return false;
}

int icsneoISO15765_DisableNetworks(void* hObject) {
	// TODO Implement
	return false;
}

int icsneoISO15765_TransmitMessage(void* hObject, unsigned long ulNetworkID, stCM_ISO157652_TxMessage* pMsg, unsigned long ulBlockingTimeout) {
	// TODO Implement
	return false;
}

int icsneoISO15765_ReceiveMessage(void* hObject, int ulNetworkID, stCM_ISO157652_RxMessage* pMsg) {
	// TODO Implement
	return false;
}

//General Utility Functions
int icsneoValidateHObject(void* hObject) {
	for(auto it = neodevices.begin(); it != neodevices.end(); it++) {
		if(&it->second == hObject) {
			neodevice_t* device = (neodevice_t*)hObject;
			if(icsneo_isValidNeoDevice(device))
				return true;
		}
	}
	
	return false;
}

int icsneoGetDLLVersion(void) {
	// TODO Implement
	return false;
}

int icsneoGetSerialNumber(void* hObject, unsigned int*iSerialNumber) {
	// TODO Implement
	return false;
}

int icsneoStartSockServer(void* hObject, int iPort) {
	// TODO Implement
	return false;
}

int icsneoStopSockServer(void* hObject) {
	// TODO Implement
	return false;
}

//CoreMini Script functions
int icsneoScriptStart(void* hObject, int iLocation) {
	// TODO Implement
	return false;
}

int icsneoScriptStop(void* hObject) {
	// TODO Implement
	return false;
}

int icsneoScriptLoad(void* hObject, const unsigned char* bin, unsigned long len_bytes, int iLocation) {
	// TODO Implement
	return false;
}

int icsneoScriptClear(void* hObject, int iLocation) {
	// TODO Implement
	return false;
}

int icsneoScriptStartFBlock(void* hObject,unsigned int fb_index) {
	// TODO Implement
	return false;
}

int icsneoScriptGetFBlockStatus(void* hObject, unsigned int fb_index, int* piRunStatus) {
	// TODO Implement
	return false;
}

int icsneoScriptStopFBlock(void* hObject,unsigned int fb_index) {
	// TODO Implement
	return false;
}

int icsneoScriptGetScriptStatus(void* hObject, int* piStatus) {
	// TODO Implement
	return false;
}

int icsneoScriptReadAppSignal(void* hObject, unsigned int iIndex, double*dValue) {
	// TODO Implement
	return false;
}

int icsneoScriptWriteAppSignal(void* hObject, unsigned int iIndex, double dValue) {
	// TODO Implement
	return false;
}

//Deprecated (but still suppored in the DLL)
int icsneoOpenPortEx(void* lPortNumber, int lPortType, int lDriverType, int lIPAddressMSB, int lIPAddressLSBOrBaudRate, int bConfigRead, unsigned char* bNetworkID, int* hObject) {
	// TODO Implement
	return false;
}

int icsneoOpenPort(int lPortNumber, int lPortType, int lDriverType, unsigned char* bNetworkID, unsigned char* bSCPIDs, int* hObject) {
	// TODO Implement
	return false;
}

int icsneoEnableNetworkCom(void* hObject, int Enable) {
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;

	if(Enable)
		return icsneo_goOnline(device);
	else
		return icsneo_goOffline(device);
}

int icsneoFindAllCOMDevices(int lDriverType, int lGetSerialNumbers, int lStopAtFirst, int lUSBCommOnly, int* p_lDeviceTypes, int* p_lComPorts, int* p_lSerialNumbers, int*lNumDevices) {
	// TODO Implement
	return false;
}