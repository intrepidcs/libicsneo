#ifndef __cplusplus
#error "icsneolegacy.cpp must be compiled with a C++ compiler!"
#endif

#define NOMINMAX

#define ICSNEOC_MAKEDLL
#include "icsneo/icsneolegacy.h"
#include "icsneo/J2534.h"

#undef ICSNEOC_MAKEDLL
#include "icsneo/icsneoc.h"

#include "icsneo/communication/network.h"
#include <map>
#include <algorithm>

#include <cstring>
#include <climits>

#ifdef _MSC_VER
#pragma warning(disable : 4100) // unreferenced formal parameter
#endif

using namespace icsneo;

typedef uint64_t legacymaphandle_t;
static std::map<legacymaphandle_t, neodevice_t> neodevices;

static std::map<const size_t, size_t> mp_netIDToVnetOffSet = {{NETID_HSCAN, 1}, {NETID_MSCAN, 2}, {NETID_HSCAN2, 18}, {NETID_HSCAN3, 19}, {NETID_HSCAN4, 32}, {NETID_HSCAN5, 33}, {NETID_HSCAN6, 47}, {NETID_HSCAN7, 48}};
static std::map<const size_t, size_t> mp_HWnetIDToCMnetID = {{NETID_HSCAN, 0}, {NETID_MSCAN, 1}, {NETID_HSCAN2, 5}, {NETID_HSCAN3, 8}, {NETID_HSCAN4, 14}, {NETID_HSCAN5, 15}, {NETID_HSCAN6, 32}, {NETID_HSCAN7, 33}};
static unsigned long vnet_table[] = {0, PLASMA_SLAVE1_OFFSET, PLASMA_SLAVE2_OFFSET};

static NeoDevice OldNeoDeviceFromNew(const neodevice_t *newnd)
{
	NeoDevice oldnd = {0};
	oldnd.DeviceType = newnd->type;
	oldnd.SerialNumber = icsneo_serialStringToNum(newnd->serial);
	oldnd.NumberOfClients = 0;
	oldnd.MaxAllowedClients = 1;
	static_assert(sizeof(neodevice_handle_t) == sizeof(oldnd.Handle), "neodevice_handle_t size must be sizeof(int) for compatibility reasons");
	oldnd.Handle = newnd->handle;
	return oldnd;
}

static void NeoMessageToSpyMessage(const neodevice_t *device, const neomessage_t &newmsg, icsSpyMessage &oldmsg)
{
	memset(&oldmsg, 0, sizeof(icsSpyMessage));
	oldmsg.NumberBytesData = (uint8_t)(newmsg.length & 0xFF);
	oldmsg.NumberBytesHeader = (uint8_t)(newmsg.length >> 8);
	oldmsg.ExtraDataPtr = (void*)newmsg.data;
	oldmsg.ExtraDataPtrEnabled = newmsg.length > 8 ? 1 : 0;
	memcpy(oldmsg.Data, newmsg.data, std::min(newmsg.length, (size_t)8));
	oldmsg.ArbIDOrHeader = *(uint32_t *)newmsg.header;
	oldmsg.NetworkID = (uint8_t)newmsg.netid; // Note: NetID remapping from the original API is not supported
	oldmsg.DescriptionID = newmsg.description;
	oldmsg.StatusBitField = newmsg.status.statusBitfield[0];
	oldmsg.StatusBitField2 = newmsg.status.statusBitfield[1];
	oldmsg.StatusBitField3 = newmsg.status.statusBitfield[2];
	oldmsg.StatusBitField4 = newmsg.status.statusBitfield[3];
	switch (Network::Type(newmsg.type))
	{
	case Network::Type::CAN:
	case Network::Type::SWCAN:
	case Network::Type::LSFTCAN:
		oldmsg.Protocol = newmsg.status.canfdFDF ? SPY_PROTOCOL_CANFD : SPY_PROTOCOL_CAN;
		break;
	case Network::Type::Ethernet:
		oldmsg.Protocol = SPY_PROTOCOL_ETHERNET;
		break;
	}

	// Timestamp - epoch = 1/1/2007 - 25ns per tick most of the time
	uint64_t t = newmsg.timestamp;
	uint16_t res = 0;
	if (icsneo_getTimestampResolution(device, &res))
	{
		t /= res;
		oldmsg.TimeHardware2 = (unsigned long)(t >> 32);
		oldmsg.TimeHardware = (unsigned long)(t & 0xFFFFFFFF);
		switch (res)
		{
		case 25:
			oldmsg.TimeStampHardwareID = HARDWARE_TIMESTAMP_ID_NEORED_25NS;
			break;
		case 10:
			oldmsg.TimeStampHardwareID = HARDWARE_TIMESTAMP_ID_NEORED_10NS;
			break;
		default:
			oldmsg.TimeStampHardwareID = HARDWARE_TIMESTAMP_ID_NONE;
		}
	}
}

static inline bool Within(size_t value, size_t min, size_t max)
{
	return ((min <= value) && (value < max));
}

static inline bool IdIsSlaveARange1(size_t fullNetid)
{
	return Within(fullNetid, PLASMA_SLAVE1_OFFSET, PLASMA_SLAVE1_OFFSET + PLASMA_SLAVE_NUM);
}

static inline bool IdIsSlaveARange2(size_t fullNetid)
{
	return Within(fullNetid, PLASMA_SLAVE1_OFFSET_RANGE2, PLASMA_SLAVE2_OFFSET_RANGE2);
}

static inline bool IdIsSlaveBRange1(size_t fullNetid)
{
	return Within(fullNetid, PLASMA_SLAVE2_OFFSET, PLASMA_SLAVE2_OFFSET + PLASMA_SLAVE_NUM);
}

static inline bool IdIsSlaveBRange2(unsigned int fullNetid)
{
	return Within(fullNetid, PLASMA_SLAVE2_OFFSET_RANGE2, PLASMA_SLAVE3_OFFSET_RANGE2);
}

static inline unsigned int GetVnetNetid(size_t simpleNetId, EPlasmaIonVnetChannel_t vnetSlot)
{
	if (vnetSlot == 0 || vnetSlot > 3)
		return simpleNetId;

	return mp_netIDToVnetOffSet[simpleNetId] + vnet_table[vnetSlot];
}

/**
 * So if you are passing in the offset from PLASMA_SLAVE1_OFFSET or
 * the offset from PLASMA_SLAVE1_OFFSET2, return the vnet agnostic
 * netid so caller can commonize handlers without caring about WHICH slave.
 */
static inline unsigned int OffsetToSimpleNetworkId(size_t offset)
{
	for (const auto& it : mp_netIDToVnetOffSet)
	{
		if (it.second == offset)
			return it.first;
	}
	return NETID_DEVICE;
}

static inline unsigned int GetVnetAgnosticNetid(size_t fullNetid)
{
	if (IdIsSlaveARange1(fullNetid))
	{
		unsigned int off = fullNetid - PLASMA_SLAVE1_OFFSET;
		return OffsetToSimpleNetworkId(off);
	}
	else if (IdIsSlaveARange2(fullNetid))
	{
		return fullNetid - PLASMA_SLAVE1_OFFSET_RANGE2;
	}
	else if (IdIsSlaveBRange1(fullNetid))
	{
		unsigned int off = fullNetid - PLASMA_SLAVE2_OFFSET;
		return OffsetToSimpleNetworkId(off);
	}
	else if (IdIsSlaveBRange2(fullNetid))
	{
		return fullNetid - PLASMA_SLAVE2_OFFSET_RANGE2;
	}
	return fullNetid;
}

//Basic Functions
int LegacyDLLExport icsneoFindDevices(NeoDeviceEx *devs, int *devCount, unsigned int *devTypes, unsigned int devTypeCount, POptionsFindNeoEx *POptionsFindNeoEx, unsigned int *zero)
{
	if (!devs || !devCount)
		return 0;

	if (*devCount < 0 || *devCount > 255)
		return 0;

	// Find the devices without filtering by the device type
	// We allow this to find more than the requested number,
	// as we may filter out some devices.
	constexpr const size_t MAX_DEVICES = 255;
	NeoDevice foundDevices[MAX_DEVICES];
	int NumDevices = MAX_DEVICES;

	int filteredDeviceCount = 0;

	if (!icsneoFindNeoDevices(0, foundDevices, &NumDevices))
		return 0;

	for (auto i = 0; i < NumDevices; i++)
	{
		// Check if the next device would overrun the user's buffer
		// We check this up here since the documentation allows zero
		// to be specified.
		if (filteredDeviceCount >= *devCount)
			break;

		if (devTypes && devTypeCount)
		{
			for (auto j = 0; j < devTypeCount; j++)
			{
				if (foundDevices[i].DeviceType == devTypes[j])
				{
					devs[filteredDeviceCount++].neoDevice = foundDevices[i];
					break;
				}
			}
		}
		else
		{
			devs[filteredDeviceCount++].neoDevice = foundDevices[i];
		}
	}

	*devCount = filteredDeviceCount;
	return 1; // If the function succeeds but no devices are found 1 will still be returned and devCount will equal 0
}

int LegacyDLLExport icsneoFindNeoDevices(unsigned long DeviceTypes, NeoDevice *pNeoDevice, int *pNumDevices)
{
	constexpr size_t MAX_DEVICES = 255;
	size_t count = MAX_DEVICES;

	if (pNumDevices == nullptr)
		return 0;

	if (pNeoDevice == nullptr)
	{
		icsneo_findAllDevices(nullptr, &count);
		*pNumDevices = (int)count;
		return 1;
	}

	size_t bufferSize = (size_t)*pNumDevices;
	if (*pNumDevices < 0 || bufferSize > MAX_DEVICES)
		return 0;

	neodevice_t devices[MAX_DEVICES];
	icsneo_findAllDevices(devices, &count);
	if (bufferSize < count)
		count = bufferSize;
	*pNumDevices = (int)count;

	for (size_t i = 0; i < count; i++)
	{
		pNeoDevice[i] = OldNeoDeviceFromNew(&devices[i]);														  // Write out into user memory
		neodevices[uint64_t(devices[i].handle) << 32 | icsneo_serialStringToNum(devices[i].serial)] = devices[i]; // Fill the look up table
	}

	return 1;
}

int LegacyDLLExport icsneoOpenNeoDevice(NeoDevice *pNeoDevice, void **hObject, unsigned char *bNetworkIDs, int bConfigRead, int bSyncToPC)
{
	if (pNeoDevice == nullptr || hObject == nullptr)
		return false;

	neodevice_t *device;
	try
	{
		device = &neodevices.at(uint64_t(pNeoDevice->Handle) << 32 | pNeoDevice->SerialNumber);
	}
	catch (const std::out_of_range&)
	{
		return false;
	}

	*hObject = device;
	if (!icsneo_openDevice(device))
		return false;
	
	return icsneo_setPollingMessageLimit(device, 20000) && icsneo_enableMessagePolling(device) && icsneo_goOnline(device);
}

int LegacyDLLExport icsneoOpenDevice(
	NeoDeviceEx* pNeoDeviceEx,
	void** hObject,
	unsigned char* bNetworkIDs,
	int bConfigRead,
	int iOptions,
	OptionsOpenNeoEx* stOptionsOpenNeoEx,
	unsigned long reserved)
{
	if (pNeoDeviceEx == nullptr || hObject == nullptr)
		return false;

	neodevice_t* device;
	try
	{
		device = &neodevices.at(uint64_t(pNeoDeviceEx->neoDevice.Handle) << 32 | pNeoDeviceEx->neoDevice.SerialNumber);
	}
	catch (const std::out_of_range&)
	{
		return false;
	}

	*hObject = device;
	if(!icsneo_openDevice(device))
		return false;
	
	return icsneo_setPollingMessageLimit(device, 20000) && icsneo_enableMessagePolling(device) && icsneo_goOnline(device);
}

int LegacyDLLExport icsneoClosePort(void *hObject, int *pNumberOfErrors)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;

	return icsneo_closeDevice(device);
}

// Memory is now managed automatically, this function is unneeded
void LegacyDLLExport icsneoFreeObject(void *hObject)
{
	(void)hObject;
	return;
}

int LegacyDLLExport icsneoSerialNumberToString(unsigned long serial, char *data, unsigned long data_size)
{
	size_t length = (size_t)data_size;
	return icsneo_serialNumToString((uint32_t)serial, data, &length);
}

//Message Functions
int LegacyDLLExport icsneoGetMessages(void *hObject, icsSpyMessage *pMsg, int *pNumberOfMessages, int *pNumberOfErrors)
{
	static neomessage_t messages[20000];
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;

	size_t messageCount = 20000;
	if (!icsneo_getMessages(device, messages, &messageCount, 0))
		return false;

	*pNumberOfMessages = (int)messageCount;
	*pNumberOfErrors = 0;

	for (size_t i = 0; i < messageCount; i++)
		NeoMessageToSpyMessage(device, messages[i], pMsg[i]);

	return true;
}

int LegacyDLLExport icsneoTxMessages(void *hObject, icsSpyMessage *pMsg, int lNetworkID, int lNumMessages)
{
	return icsneoTxMessagesEx(hObject, pMsg, lNetworkID, lNumMessages, nullptr, 0);
}

int LegacyDLLExport icsneoTxMessagesEx(void *hObject, icsSpyMessage *pMsg, unsigned int lNetworkID, unsigned int lNumMessages, unsigned int *NumTxed, unsigned int zero2)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	neomessage_t newmsg;
	unsigned int temp = 0;
	if (NumTxed == nullptr)
		NumTxed = &temp;
	*NumTxed = 0;
	for (unsigned int i = 0; i < lNumMessages; i++)
	{
		const icsSpyMessage &oldmsg = pMsg[i];
		newmsg = {};
		newmsg.netid = (uint16_t)lNetworkID;
		newmsg.description = oldmsg.DescriptionID;
		memcpy(newmsg.header, &oldmsg.ArbIDOrHeader, sizeof(newmsg.header));
		if (oldmsg.Protocol != SPY_PROTOCOL_ETHERNET)
			newmsg.length = oldmsg.NumberBytesData;
		else
			newmsg.length = ((oldmsg.NumberBytesHeader & 255) << 8) | (oldmsg.NumberBytesData & 255);
		if (oldmsg.ExtraDataPtr != nullptr && oldmsg.ExtraDataPtrEnabled == 1)
			newmsg.data = reinterpret_cast<const uint8_t *>(oldmsg.ExtraDataPtr);
		else
			newmsg.data = oldmsg.Data;
		newmsg.status.statusBitfield[0] = oldmsg.StatusBitField;
		newmsg.status.statusBitfield[1] = oldmsg.StatusBitField2;
		newmsg.status.statusBitfield[2] = oldmsg.StatusBitField3;
		newmsg.status.statusBitfield[3] = oldmsg.StatusBitField4;
		if (oldmsg.Protocol == SPY_PROTOCOL_CANFD)
			newmsg.status.canfdFDF = true;
		if (icsneo_transmit(device, &newmsg))
			(*NumTxed)++;
	}
	return lNumMessages == *NumTxed;
}

int LegacyDLLExport icsneoEnableNetworkRXQueue(void *hObject, int iEnable)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetTimeStampForMsg(void* hObject, icsSpyMessage* pMsg, double* pTimeStamp) {
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;

	uint16_t resolution = 0;
	if (!icsneo_getTimestampResolution(device, &resolution))
		return false;

	// Convert back to ticks
	uint64_t ticks = pMsg->TimeHardware2;
	ticks <<= 32;
	ticks += pMsg->TimeHardware;

	// convert to ns
	ticks *= resolution;

	// icsneoGetTimeStampForMsg() expects pTimeStamp to be in seconds
	*pTimeStamp = ticks / (double)1000000000;

	return true;
}

void LegacyDLLExport icsneoGetISO15765Status(void *hObject, int lNetwork, int lClearTxStatus, int lClearRxStatus, int *lTxStatus, int *lRxStatus)
{
	// TODO Implement
	return;
}

void LegacyDLLExport icsneoSetISO15765RxParameters(void *hObject, int lNetwork, int lEnable, spyFilterLong *pFF_CFMsgFilter, icsSpyMessage *pTxMsg,
												   int lCFTimeOutMs, int lFlowCBlockSize, int lUsesExtendedAddressing, int lUseHardwareIfPresent)
{
	// TODO Implement
	return;
}

//Device Functions
int LegacyDLLExport icsneoGetConfiguration(void *hObject, unsigned char *pData, int *lNumBytes)
{
	// 2G devices are not supported in the new API
	return false;
}

int LegacyDLLExport icsneoSendConfiguration(void *hObject, unsigned char *pData, int lNumBytes)
{
	// 2G devices are not supported in the new API
	return false;
}

int LegacyDLLExport icsneoGetFireSettings(void *hObject, SFireSettings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetFireSettings(void *hObject, SFireSettings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCAN3Settings(void *hObject, SVCAN3Settings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCAN3Settings(void *hObject, SVCAN3Settings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetFire2Settings(void *hObject, SFire2Settings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetFire2Settings(void *hObject, SFire2Settings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCANRFSettings(void *hObject, SVCANRFSettings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCANRFSettings(void *hObject, SVCANRFSettings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCAN412Settings(void *hObject, SVCAN412Settings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCAN412Settings(void *hObject, SVCAN412Settings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetRADGalaxySettings(void *hObject, SRADGalaxySettings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetRADGalaxySettings(void *hObject, SRADGalaxySettings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetRADStar2Settings(void *hObject, SRADStar2Settings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetRADStar2Settings(void *hObject, SRADStar2Settings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoSetBitRate(void *hObject, int BitRate, int NetworkID)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (!icsneo_setBaudrate(device, (uint16_t)NetworkID, BitRate))
		return false;
	return icsneo_settingsApply(device);
}

int LegacyDLLExport icsneoSetFDBitRate(void* hObject, int BitRate, int NetworkID) {
    if(!icsneoValidateHObject(hObject))
        return false;
    neodevice_t* device = (neodevice_t*)hObject;
    if(!icsneo_setFDBaudrate(device, (uint16_t)NetworkID, BitRate))
        return false;
    return icsneo_settingsApply(device);
}

int LegacyDLLExport icsneoGetDeviceParameters(void* hObject, char* pParameter, char* pValues, short ValuesLength) {
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoSetDeviceParameters(void *hObject, char *pParmValue, int *pErrorIndex, int bSaveToEEPROM)
{
	// TODO Implement
	return false;
}

//Error Functions
int LegacyDLLExport icsneoGetLastAPIError(void *hObject, unsigned long *pErrorNumber)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetErrorMessages(void *hObject, int *pErrorMsgs, int *pNumberOfErrors)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetErrorInfo(int lErrorNumber, TCHAR *szErrorDescriptionShort, TCHAR *szErrorDescriptionLong, int *lMaxLengthShort, int *lMaxLengthLong, int *lErrorSeverity, int *lRestartNeeded)
{
	// TODO Implement
	return false;
}

//ISO15765-2 Functions
int LegacyDLLExport icsneoISO15765_EnableNetworks(void *hObject, unsigned long ulNetworks)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_DisableNetworks(void *hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_TransmitMessage(void *hObject, unsigned long ulNetworkID, stCM_ISO157652_TxMessage *pMsg, unsigned long ulBlockingTimeout)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_ReceiveMessage(void *hObject, int ulNetworkID, stCM_ISO157652_RxMessage *pMsg)
{
	// TODO Implement
	return false;
}

//General Utility Functions
int LegacyDLLExport icsneoValidateHObject(void *hObject)
{
	for (auto it = neodevices.begin(); it != neodevices.end(); it++)
	{
		if (&it->second == hObject)
		{
			neodevice_t *device = (neodevice_t *)hObject;
			if (icsneo_isValidNeoDevice(device))
				return true;
		}
	}

	return false;
}

int LegacyDLLExport icsneoGetDLLVersion(void)
{
	return 804;
}

int LegacyDLLExport icsneoGetSerialNumber(void *hObject, unsigned int *iSerialNumber)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	*iSerialNumber = icsneo_serialStringToNum(device->serial);
	return true;
}

int LegacyDLLExport icsneoEnableDOIPLine(void* hObject, bool enable)
{
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	return icsneo_setDigitalIO(device, ICSNEO_IO_ETH_ACTIVATION, 1, enable);
}

int LegacyDLLExport icsneoStartSockServer(void* hObject, int iPort)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoStopSockServer(void *hObject)
{
	// TODO Implement
	return false;
}

//CoreMini Script functions
int LegacyDLLExport icsneoScriptStart(void *hObject, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStop(void *hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptLoad(void *hObject, const unsigned char *bin, unsigned long len_bytes, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptClear(void *hObject, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStartFBlock(void *hObject, unsigned int fb_index)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptGetFBlockStatus(void *hObject, unsigned int fb_index, int *piRunStatus)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStopFBlock(void *hObject, unsigned int fb_index)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptGetScriptStatus(void *hObject, int *piStatus)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptReadAppSignal(void *hObject, unsigned int iIndex, double *dValue)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptWriteAppSignal(void *hObject, unsigned int iIndex, double dValue)
{
	// TODO Implement
	return false;
}

//Deprecated (but still suppored in the DLL)
int LegacyDLLExport icsneoOpenPortEx(void *lPortNumber, int lPortType, int lDriverType, int lIPAddressMSB, int lIPAddressLSBOrBaudRate, int bConfigRead, unsigned char *bNetworkID, int *hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoOpenPort(int lPortNumber, int lPortType, int lDriverType, unsigned char *bNetworkID, unsigned char *bSCPIDs, int *hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoEnableNetworkCom(void *hObject, int Enable)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;

	if (Enable)
		return icsneo_goOnline(device);
	else
		return icsneo_goOffline(device);
}

int LegacyDLLExport icsneoFindAllCOMDevices(int lDriverType, int lGetSerialNumbers, int lStopAtFirst, int lUSBCommOnly, int *p_lDeviceTypes, int *p_lComPorts, int *p_lSerialNumbers, int *lNumDevices)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoOpenNeoDeviceByChannels(NeoDevice *pNeoDevice, void **hObject, unsigned char *uChannels, int iSize, int bConfigRead, int iOptions)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetVCAN4Settings(void *hObject, SVCAN4Settings *pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	return !!icsneo_settingsReadStructure(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoSetVCAN4Settings(void *hObject, SVCAN4Settings *pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetDeviceSettingsType(void *hObject, EPlasmaIonVnetChannel_t vnetSlot, EDeviceSettingsType *pDeviceSettingsType)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t *device = (neodevice_t *)hObject;

	unsigned long ulDeviceType = device->type;

	if (pDeviceSettingsType == nullptr)
		return 0;

	if ((PlasmaIonVnetChannelMain != vnetSlot) && (ulDeviceType != NEODEVICE_PLASMA) && (ulDeviceType != NEODEVICE_ION))
		return 0;

	switch (ulDeviceType)
	{
	case NEODEVICE_PLASMA:
	case NEODEVICE_ION:
		*pDeviceSettingsType = DeviceFire2SettingsType; //defaults to FIRE2 vnets with libicsneo - no firevnets!
		break;
	case NEODEVICE_VCAN3:
		*pDeviceSettingsType = DeviceVCAN3SettingsType;
		break;
	case NEODEVICE_FIRE:
		*pDeviceSettingsType = DeviceFireSettingsType;
		break;
	case NEODEVICE_FIRE2:
		*pDeviceSettingsType = DeviceFire2SettingsType;
		break;
	case NEODEVICE_RADGALAXY:
		*pDeviceSettingsType = DeviceRADGalaxySettingsType;
		break;
	case NEODEVICE_RADSTAR2:
		*pDeviceSettingsType = DeviceRADStar2SettingsType;
		break;
	case NEODEVICE_VCAN44:
	case NEODEVICE_VCAN42_EL:
		*pDeviceSettingsType = DeviceVCAN4SettingsType;
		break;
	case NEODEVICE_VCAN41:
	case NEODEVICE_VCAN42:
		*pDeviceSettingsType = DeviceVCAN412SettingsType;
		break;
	case NEODEVICE_VIVIDCAN:
		*pDeviceSettingsType = DeviceVividCANSettingsType;
		break;
	case NEODEVICE_ECU_AVB:
		*pDeviceSettingsType = DeviceECU_AVBSettingsType;
		break;
	case NEODEVICE_RADSUPERMOON:
		*pDeviceSettingsType = DeviceRADSuperMoonSettingsType;
		break;
	case NEODEVICE_RADMOON2:
		*pDeviceSettingsType = DeviceRADMoon2SettingsType;
		break;
	case NEODEVICE_RADGIGALOG:
		*pDeviceSettingsType = DeviceRADGigalogSettingsType;
		break;
	default:
		return 0;
	}

	return 1;
}

int LegacyDLLExport icsneoSetDeviceSettings(void* hObject, SDeviceSettings* pSettings, int iNumBytes, int bSaveToEEPROM, EPlasmaIonVnetChannel_t vnetSlot)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = (neodevice_t*)hObject;

	const size_t offset = size_t(&pSettings->Settings) - size_t(pSettings);

	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, &pSettings->Settings, iNumBytes - offset);
	else
		return icsneo_settingsApplyStructureTemporary(device, &pSettings->Settings, iNumBytes - offset);
}

int LegacyDLLExport icsneoGetDeviceSettings(void* hObject, SDeviceSettings* pSettings, int iNumBytes, EPlasmaIonVnetChannel_t vnetSlot)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = (neodevice_t*)hObject;

	if (icsneoGetDeviceSettingsType(hObject, vnetSlot, &pSettings->DeviceSettingType) == 0)
		return false;

	const size_t offset = size_t(&pSettings->Settings) - size_t(pSettings);
	return !!icsneo_settingsReadStructure(device, &pSettings->Settings, iNumBytes - offset);
}

int LegacyDLLExport icsneoSetBitRateEx(void *hObject, unsigned long BitRate, int NetworkID, int iOptions)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t *device = (neodevice_t *)hObject;
	if (!icsneo_setBaudrate(device, (uint16_t)NetworkID, BitRate))
		return false;

	bool bSaveToEEPROM = iOptions & 0x1;

	if (bSaveToEEPROM)
		return icsneo_settingsApply(device);
	else
		return icsneo_settingsApplyTemporary(device);
}

int LegacyDLLExport icsneoEnableNetworkComEx(void *hObject, int iEnable, int iNetId)
{
	return icsneoEnableNetworkCom(hObject, iEnable);
}

int LegacyDLLExport icsneoForceFirmwareUpdate(void *hObject)
{
	return false;
}

int LegacyDLLExport icsneoGetHWFirmwareInfo(void *hObject, stAPIFirmwareInfo *pInfo)
{
	return false;
}

int LegacyDLLExport icsneoSetUSBLatency(NeoDevice *pNeoDevice, unsigned long LatencyVal)
{
	return false;
}

/**
 * @fn	unsigned int _stdcall icsneoGetCANControllerClockFrequency(int hObject,
 * int lNetworkID)
 *
 * @brief	Gets the CAN Controller clock frequency of a specified CAN
 * network.  For most cases this is 20Mhz (40 MIPS/2).  The clock frequency
 * is dependent on the network and product.  This function is necessary when
 * computing the time quanta parameters of a CAN network, specifically the
 * Bit Rate Prescaler (BRP). As a general rule if you are using
 * icsneoSetCANParameters then you should also be calling this function.
 *
 * @param	hObject		Handle to Hardware, see OpenNeoDevice.
 * @param	lNetworkID	The CAN network to retrieve Clock period from.
 *
 * @return	Clock Period of specified CAN Controller, or 0 if input
 * parameters are invalid.
 */
unsigned int LegacyDLLExport icsneoGetCANControllerClockFrequency(void* hObject, int lNetworkID)
{
	return false;
}

/**
* @fn	int _stdcall icsneoSetCANParameters(int hObject,
* unsigned int baudRatePrescaler, unsigned int propagationTqs,
* unsigned int phase1Tqs, unsigned int phase2Tqs,
* unsigned int syncJumpWidthTqs, unsigned int optionBits, int lNetworkID)
*
* @brief	Sets the timing parameters of a specific CAN network.
*
* @param	hObject				Handle to Hardware, see OpenNeoDevice.
* @param	baudRatePrescaler	The baud rate prescaler[1-63].  Zero is
* 								invalid.
* @param	propagationTqs		The propagation delay in time quantas.
* @param	phase1Tqs			The phase 1 segment in time quantas.
* @param	phase2Tqs			The phase 2 segment in time quantas.
* @param	syncJumpWidthTqs	The synchronisation jump width in time
* 								quantas.
* @param	optionBits			Reserved, set to zero.
* @param	lNetworkID			The CAN network to retrieve Clock period
* 								from.
*
* @return	1 iff successful, 0 otherwise.
*/
int LegacyDLLExport icsneoSetCANParameters(void* hObject,
	unsigned int baudRatePrescaler,
	unsigned int propagationTqs,
	unsigned int phase1Tqs,
	unsigned int phase2Tqs,
	unsigned int syncJumpWidthTqs,
	unsigned int optionBits,
	int lNetworkID)
{
	return false;
}

int LegacyDLLExport icsneoSetCANParametersPhilipsSJA1000(void* hObject,
	unsigned int btr0,
	unsigned int btr1,
	unsigned int optionBits,
	int lNetworkID)
{
	return false;
}

int LegacyDLLExport icsneoEnableBitSmash(void *hObject,
										 int netId,
										 unsigned int numWaitBits,
										 unsigned int numSmashBits,
										 unsigned int numFrames,
										 unsigned int timeout_ms,
										 unsigned int optionBits,
										 unsigned int numArbIds,
										 unsigned int arbIds[MAX_BIT_SMASH_ARBIDS])
{
	return false;
}

int LegacyDLLExport icsneoDisableBitSmash(void *hObject, unsigned int reservedZero)
{
	return false;
}

int LegacyDLLExport icsneoSendHWTimeRequest(void *hObject)
{
	return false;
}

int LegacyDLLExport icsneoReceiveHWTimeResponse(void *hObject, double *TimeHardware, unsigned long TimeOut)
{
	return false;
}

int LegacyDLLExport icsneoGetDLLFirmwareInfo(void *hObject, stAPIFirmwareInfo *pInfo)
{
	return false;
}

int LegacyDLLExport icsneoGetDLLFirmwareInfoEx(void *hObject, stAPIFirmwareInfo *pInfo, EPlasmaIonVnetChannel_t vnetSlot)
{
	return false;
}

int LegacyDLLExport icsneoJ2534Cmd(void *hObject, unsigned char *CmdBuf, short Len, void *pVoid)
{
	uint64_t* pTmp = nullptr;
	int iRetVal = 0, iNumBytes = 0, NetworkID;

	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t *device = (neodevice_t *)hObject;

	switch (*CmdBuf)
	{
	case J2534NVCMD_SetNetworkBaudRate:

		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		if (!icsneo_setBaudrate(device, NetworkID, *pTmp) && !icsneo_settingsApply(device))
			iRetVal = 0;
		break;

	case J2534NVCMD_GetNetworkBaudRate:
	{
		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		//Ignoring 2G hardwares here - CmdBuf[9]
		int64_t ret = icsneo_getBaudrate(device, NetworkID);
		if (ret < 0)
			return false;
		*pTmp = static_cast<uint64_t>(ret);
		break;
	}
	case J2534NVCMD_SetCANFDRate:

		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		//Ignoring 2G hardwares here - CmdBuf[9]
		if (!icsneo_setFDBaudrate(device, NetworkID, *pTmp) && !icsneo_settingsApply(device))
			iRetVal = 0;
		break;

	case J2534NVCMD_GetCANFDRate:

		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		//Ignoring 2G hardwares here - CmdBuf[9]
		*pTmp = icsneo_getFDBaudrate(device, NetworkID);
		break;

	case J2534NVCMD_GetCANFDTermination:

		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		switch (device->type)
		{
		case NEODEVICE_FIRE2:
		case NEODEVICE_PLASMA: //FIRE2 VNETS
		case NEODEVICE_ION:	//FIRE2 VNETS
			SFire2Settings Cs;
			iNumBytes = sizeof(Cs);
			if (!!icsneoGetFire2Settings(hObject, &Cs, iNumBytes))
				(Cs.termination_enables & (1ull << mp_HWnetIDToCMnetID[GetVnetAgnosticNetid(NetworkID)])) ? *pTmp = 3 /*Termination ON*/ : *pTmp = 0 /*Termination OFF*/;
			else
				iRetVal = 0;
			break;
		}
		break;

	case J2534NVCMD_SetCANFDTermination:

		pTmp = (uint64_t *)&CmdBuf[1];
		NetworkID = (uint16_t)*pTmp;
		pTmp = (uint64_t *)&CmdBuf[5];

		switch (device->type)
		{
		case NEODEVICE_FIRE2:
		case NEODEVICE_PLASMA:
		case NEODEVICE_ION:
			SFire2Settings Cs;
			iNumBytes = sizeof(Cs);
			if (icsneoGetFire2Settings(hObject, &Cs, iNumBytes))
			{
				unsigned long long CoremininetID = mp_HWnetIDToCMnetID[GetVnetAgnosticNetid(NetworkID)];
			
				if (*pTmp == 3) /*Termination ON*/
					Cs.termination_enables |= (1ull << CoremininetID);
				else /*Termination OFF*/
					Cs.termination_enables &= ~(1ull << CoremininetID);

				iRetVal = icsneoSetFire2Settings(hObject, &Cs, iNumBytes, 1 /* ConfigurationOptionDoNotSaveToEEPROM */);
			}
			break;
		}
		break;

	default:
		break;
	}
	return iRetVal;
}

int LegacyDLLExport icsneoSendRawCmds(void *hObject, icsneoVICommand *pCmdMsgs, int lNumOfCmds)
{
	return false;
}

int LegacyDLLExport icsneoEnableBusVoltageMonitor(void *hObject, unsigned int enable, unsigned int reserved)
{
	return false;
}

int LegacyDLLExport icsneoISO15765_TransmitMessageEx(void *hObject,
													 unsigned long ulNetworkID,
													 ISO15765_2015_TxMessage *pMsg,
													 unsigned long ulBlockingTimeout)
{
	return false;
}

int LegacyDLLExport icsneoGetBusVoltage(void *hObject, unsigned long *pVBusVoltage, unsigned int reserved)
{
	return false;
}

int LegacyDLLExport icsneoOpenRemoteNeoDevice(const char *pIPAddress,
											  NeoDevice *pNeoDevice,
											  void **hObject,
											  unsigned char *bNetworkIDs,
											  int iOptions)
{
	return false;
}

int LegacyDLLExport icsneoFindRemoteNeoDevices(const char *pIPAddress, NeoDevice *pNeoDevice, int *pNumDevices)
{
	return false;
}

int LegacyDLLExport icsneoFirmwareUpdateRequired(void *hObject)
{
	return false;
}

void LegacyDLLExport icsneoGetDLLVersionEx(unsigned long *dwMSVersion, unsigned long *dwLSVersion)
{
	return;
}

int LegacyDLLExport icsneoGetNetidforSlaveVNETs(size_t *NetworkIndex, EPlasmaIonVnetChannel_t vnetSlot)
{
	return GetVnetNetid(*NetworkIndex, vnetSlot);
}

int LegacyDLLExport icsneoGetVnetSimpleNetid(size_t *FullNetID)
{
	return GetVnetAgnosticNetid(*FullNetID);
}

int LegacyDLLExport icsneoSerialNumberFromString(unsigned long *serial, char *data)
{
	if (serial == nullptr)
		return false;

	*serial = icsneo_serialStringToNum(data);
	return false;
}

int LegacyDLLExport icsneoGetMiniportAdapterInfo(void *hObject, NETWORK_ADAPTER_INFO *aInfo)
{
	return false;
}
