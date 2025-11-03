#ifndef __cplusplus
#error "icsneolegacy.cpp must be compiled with a C++ compiler!"
#endif

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
#pragma warning(disable : 4996) // STL time functions
#endif

using namespace icsneo;

typedef uint64_t legacymaphandle_t;
static std::map<legacymaphandle_t, neodevice_t> neodevices;
static std::map<neodevice_t*, NeoDeviceEx*> openneodevices;

static const std::map<size_t, size_t> mp_netIDToVnetOffSet = {
	{NETID_DWCAN_01, 1},
	{NETID_DWCAN_08, 2},
	{NETID_DWCAN_02, 18},
	{NETID_DWCAN_03, 19},
	{NETID_DWCAN_04, 32},
	{NETID_DWCAN_05, 33},
	{NETID_DWCAN_06, 47},
	{NETID_DWCAN_07, 48},
};

static const std::map<size_t, size_t> mp_HWnetIDToCMnetID = {
	{NETID_DWCAN_01, 0},
	{NETID_DWCAN_08, 1},
	{NETID_DWCAN_02, 5},
	{NETID_DWCAN_03, 8},
	{NETID_DWCAN_04, 14},
	{NETID_DWCAN_05, 15},
	{NETID_DWCAN_06, 32},
	{NETID_DWCAN_07, 33},
};

static unsigned long vnet_table[] = {0, PLASMA_SLAVE1_OFFSET, PLASMA_SLAVE2_OFFSET};


static bool NeoMessageToSpyMessage(const neodevice_t* device, const neomessage_t& newmsg, icsSpyMessage& oldmsg)
{
	memset(&oldmsg, 0, sizeof(icsSpyMessage));

	// We currently only deal with frames
	if (newmsg.messageType != ICSNEO_MESSAGE_TYPE_FRAME)
		return false;

	const neomessage_frame_t& frame = *reinterpret_cast<const neomessage_frame_t*>(&newmsg);

	auto copyStatusData = [&]() {
		oldmsg.NetworkID = static_cast<uint8_t>(frame.netid); // Note: NetID remapping from the original API is not supported
		oldmsg.NetworkID2 = static_cast<uint8_t>(frame.netid >> 8);
		oldmsg.DescriptionID = frame.description;
		oldmsg.StatusBitField = frame.status.statusBitfield[0];
		oldmsg.StatusBitField2 = frame.status.statusBitfield[1];
		oldmsg.StatusBitField3 = frame.status.statusBitfield[2];
		oldmsg.StatusBitField4 = frame.status.statusBitfield[3];
	};

	auto copyFrameData = [&]() {
		oldmsg.ExtraDataPtr = (void*)frame.data;
		oldmsg.ExtraDataPtrEnabled = frame.length > 8 ? 1 : 0;
		memcpy(oldmsg.Data, frame.data, std::min(frame.length, (size_t)8));
		oldmsg.ArbIDOrHeader = *reinterpret_cast<const uint32_t*>(frame.header);
		copyStatusData();
	};

	switch (Network::Type(frame.type))
	{
	case Network::Type::CAN:
	case Network::Type::SWCAN:
	case Network::Type::LSFTCAN:
		oldmsg.Protocol = frame.status.canfdFDF ? SPY_PROTOCOL_CANFD : SPY_PROTOCOL_CAN;
		oldmsg.NumberBytesData = static_cast<uint8_t>(std::min(frame.length, (size_t)255));
		oldmsg.NumberBytesHeader = 4;
		copyFrameData();
		break;
	case Network::Type::Ethernet:
	case Network::Type::AutomotiveEthernet:
		oldmsg.Protocol = SPY_PROTOCOL_ETHERNET;
		oldmsg.NumberBytesData = static_cast<uint8_t>(frame.length & 0xFF);
		oldmsg.NumberBytesHeader = static_cast<uint8_t>(frame.length >> 8);
		copyFrameData();
		break;
	case Network::Type::LIN:
	{
		const neomessage_lin_t& linFrame = *reinterpret_cast<const neomessage_lin_t*>(&frame);
		icsSpyMessageJ1850& linSpyMsg = *reinterpret_cast<icsSpyMessageJ1850*>(&oldmsg);
		linSpyMsg.Protocol = SPY_PROTOCOL_LIN;
		linSpyMsg.NumberBytesHeader = static_cast<uint8_t>(std::min(linFrame.length, static_cast<size_t>(3)));
		linSpyMsg.NumberBytesData = static_cast<uint8_t>(linFrame.length - linSpyMsg.NumberBytesHeader);
		oldmsg.ArbIDOrHeader = *reinterpret_cast<const uint32_t*>(frame.header);
		copyStatusData();
		if ((2 < linFrame.length) && (linFrame.length <= 10)) {
			auto copyBytes = std::min(linSpyMsg.NumberBytesData, static_cast<uint8_t>(6));
			std::memcpy(oldmsg.Data, frame.data, copyBytes);
			oldmsg.Data[copyBytes] = linFrame.checksum;
		} else if (2 == linFrame.length) {
			std::memset(oldmsg.Data, 0, 8);
			linSpyMsg.Header[linSpyMsg.NumberBytesHeader] = linFrame.checksum;
			++linSpyMsg.NumberBytesHeader;
		} else {
			std::memset(oldmsg.Data, 0, 8);
		}
		if (linFrame.linStatus.txCommander)
			linSpyMsg.StatusBitField |= SPY_STATUS_INIT_MESSAGE;
		break;
	}
	default:
		return false;
	}

	// Timestamp - epoch = 1/1/2007 - 25ns per tick most of the time
	uint64_t t = frame.timestamp;
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

	return true;
}

static bool SpyMessageToNeoMessage(const icsSpyMessage& oldmsg, neomessage_frame_t& frame, unsigned int& lNetworkID)
{
	frame.netid = static_cast<uint16_t>(lNetworkID);
	frame.description = oldmsg.DescriptionID;

	frame.status.statusBitfield[0] = oldmsg.StatusBitField;
	frame.status.statusBitfield[1] = oldmsg.StatusBitField2;
	frame.status.statusBitfield[2] = oldmsg.StatusBitField3;
	frame.status.statusBitfield[3] = oldmsg.StatusBitField4;

	auto copyFrameDataPtr = [&]() {
		memcpy(frame.header, &oldmsg.ArbIDOrHeader, sizeof(frame.header));
		if ((oldmsg.ExtraDataPtr != nullptr) && (oldmsg.ExtraDataPtrEnabled == 1))
			frame.data = reinterpret_cast<const uint8_t *>(oldmsg.ExtraDataPtr);
		else
			frame.data = oldmsg.Data;
	};

	switch(oldmsg.Protocol)
	{
		case SPY_PROTOCOL_ETHERNET:
		{
			frame.length = ((oldmsg.NumberBytesHeader & 255) << 8) | (oldmsg.NumberBytesData & 255);
			copyFrameDataPtr();
			break;
		}
		case SPY_PROTOCOL_LIN:
		{
			neomessage_lin_t& linFrame = *reinterpret_cast<neomessage_lin_t*>(&frame);
			const uint8_t numberBytesHeader = std::min(oldmsg.NumberBytesHeader, static_cast<uint8_t>(3));
			const uint8_t numberBytesData = std::min(oldmsg.NumberBytesData, static_cast<uint8_t>(7));
			frame.length = numberBytesHeader + numberBytesData;
			linFrame.type = ICSNEO_NETWORK_TYPE_LIN;
			if (oldmsg.StatusBitField & SPY_STATUS_INIT_MESSAGE)
				linFrame.linStatus.txCommander = true;
			else
				linFrame.linStatus.txResponder = true;

			copyFrameDataPtr();
			uint8_t protID = linFrame.header[0] & 0x3Fu;
			auto bit = [&](uint8_t pos)->uint8_t { return ((protID >> pos) & 0x1u); };
			protID |= (~(bit(1) ^ bit(3) ^ bit(4) ^ bit(5)) << 7);
			protID |= ((bit(0) ^ bit(1) ^ bit(2) ^ bit(4)) << 6);
			linFrame.header[0] = protID;
			if (frame.length > 2) {
				size_t checksum = 0;
				uint8_t* lastByte = nullptr;
				for(size_t idx = 1; idx < (frame.length - 1); ++idx) {
					if (idx < oldmsg.NumberBytesHeader) {
						checksum += frame.header[idx];
						lastByte = (frame.header + idx + 1);
					} else {
						checksum += frame.data[idx-numberBytesHeader];
						lastByte = const_cast<uint8_t*>(frame.data) + idx - numberBytesHeader + 1;
					}
					if (checksum > 255) { checksum -= 255; }
				}
				size_t enhanced = frame.header[0] + checksum;
				if (enhanced > 255) { enhanced -= 255; }
				checksum ^= 0xff;
				enhanced ^= 0xff;

				if ((lastByte != nullptr) && (*lastByte != checksum) && (*lastByte == enhanced))
					linFrame.linStatus.txChecksumEnhanced = true;
			}
			break;
		}
		case SPY_PROTOCOL_CANFD:
		{
			frame.length = oldmsg.NumberBytesData;
			frame.status.canfdFDF = true;
			copyFrameDataPtr();
			break;
		}
		default:
		{
			frame.length = oldmsg.NumberBytesData;
			copyFrameDataPtr();
			break;
		}
	} 
	return true;
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

static inline bool IdIsSlaveBRange2(size_t fullNetid)
{
	return Within(fullNetid, PLASMA_SLAVE2_OFFSET_RANGE2, PLASMA_SLAVE3_OFFSET_RANGE2);
}

static inline bool GetVnetNetid(size_t& netId, EPlasmaIonVnetChannel_t vnetSlot)
{
	if (vnetSlot == 0)
		return true;

	if (vnetSlot >= sizeof(vnet_table)/sizeof(vnet_table[0]))
		return false;

	const auto offset = mp_netIDToVnetOffSet.find(netId);
	if (offset == mp_netIDToVnetOffSet.end())
		return false;

	netId = offset->second + vnet_table[vnetSlot];
	return true;
}

/**
 * So if you are passing in the offset from PLASMA_SLAVE1_OFFSET or
 * the offset from PLASMA_SLAVE1_OFFSET2, return the vnet agnostic
 * netid so caller can commonize handlers without caring about WHICH slave.
 */
static inline size_t OffsetToSimpleNetworkId(size_t offset)
{
	for (const auto& it : mp_netIDToVnetOffSet)
	{
		if (it.second == offset)
			return it.first;
	}
	return NETID_DEVICE;
}

static inline size_t GetVnetAgnosticNetid(size_t fullNetid)
{
	if (IdIsSlaveARange1(fullNetid))
		return OffsetToSimpleNetworkId(fullNetid - PLASMA_SLAVE1_OFFSET);
	else if (IdIsSlaveARange2(fullNetid))
		return fullNetid - PLASMA_SLAVE1_OFFSET_RANGE2;
	else if (IdIsSlaveBRange1(fullNetid))
		return OffsetToSimpleNetworkId(fullNetid - PLASMA_SLAVE2_OFFSET);
	else if (IdIsSlaveBRange2(fullNetid))
		return fullNetid - PLASMA_SLAVE2_OFFSET_RANGE2;
	return fullNetid;
}

//Basic Functions
int LegacyDLLExport icsneoFindDevices(NeoDeviceEx* devs, int* devCount, unsigned int* devTypes, unsigned int devTypeCount,
	POptionsFindNeoEx* POptionsFindNeoEx, unsigned int* zero)
{
	// Match the legacy API maximum, this derives from the maximum COM Port on old windows versions.
	constexpr int MAX_NEO_DEVICES = 255;

	// Validate arguments
	if (!devCount && *devCount < 0) {
		return 0;
	}
	// return the size only if devs is NULL.
	if (!devs)
	{
		icsneo_findAllDevices(nullptr, (size_t*)devCount);
		return 1;
	}
	// shrink the number of devices allowed to find
	if (*devCount > MAX_NEO_DEVICES) {
		*devCount = MAX_NEO_DEVICES;
	}
	// Find all the neodevice_t devices
	std::vector<neodevice_t> neoDevices(*devCount);
	auto neoDevicesSize = neoDevices.size();
	icsneo_findAllDevices(neoDevices.data(), &neoDevicesSize);
	neoDevices.resize(neoDevicesSize);
	// Filter out the devices if needed
	// No filtering needed
	if (devTypes && devTypeCount > 0) {
		neoDevices.erase(
			std::remove_if(
				neoDevices.begin(), 
				neoDevices.end(), 
				[&](const auto& iter) {
					for (unsigned int i=0; i < devTypeCount; ++i) {
						if (iter.type == devTypes[i]) {
							return false;
						}
					}
					return true;
				}
			),
			neoDevices.end()
		);
	}
	// Create a NeoDeviceEx From a neodevice_t
	auto _createNeoDeviceExFrom = [](const neodevice_t* neoDevice) -> NeoDeviceEx {
		NeoDeviceEx nde = {};
		nde.neoDevice.DeviceType = neoDevice->type;
		nde.neoDevice.SerialNumber = icsneo_serialStringToNum(neoDevice->serial);
		nde.neoDevice.NumberOfClients = 0;
		nde.neoDevice.MaxAllowedClients = 1;
		static_assert(sizeof(neodevice_handle_t) == sizeof(nde.neoDevice.Handle),
			"neodevice_handle_t size must be sizeof(int) for compatibility reasons");
		nde.neoDevice.Handle = neoDevice->handle;
		return nde;
	};
	
	// Create the NeoDeviceEx from the neodevice_t
	auto i = 0;
	for (const auto& neoDevice : neoDevices) {
		// Fill the look up table
		neodevices[uint64_t(neoDevice.handle) << 32 | icsneo_serialStringToNum(neoDevice.serial)] = neoDevice;
		// Create the NeoDeviceEx
		devs[i] = _createNeoDeviceExFrom(&neoDevice);
		NeoDeviceEx* nde = &devs[i];
		++i;
		// Lookup the open NeoDeviceEx devices and match the NumberOfClients value if available.
		for (auto& [neo_device, open_nde]: openneodevices) {
			// SerialNumber should always be unique so lets compare against that.
			if (nde->neoDevice.SerialNumber == open_nde->neoDevice.SerialNumber) {
				nde->neoDevice.NumberOfClients = open_nde->neoDevice.NumberOfClients;
			}
		}
	}
	*devCount = (int)neoDevices.size();
	return 1;
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

	if (pNeoDeviceEx->neoDevice.NumberOfClients >= pNeoDeviceEx->neoDevice.MaxAllowedClients) {
		return false;
	}

	*hObject = device;
	if(!icsneo_openDevice(device)) {
		return false;
	}
	
	if (icsneo_isOnlineSupported(device)) {
		if (!icsneo_setPollingMessageLimit(device, 20000)) {
			icsneo_closeDevice(device);
			return false;
		}

		if (!icsneo_enableMessagePolling(device)) {
			icsneo_closeDevice(device);
			return false;
		}
		
		if (!icsneo_goOnline(device)) {
			icsneo_closeDevice(device);
			return false;
		}
	}
	pNeoDeviceEx->neoDevice.NumberOfClients = 1;
	// Add the open NeoDevice to the container so we can decrement NumberOfClients on close
	openneodevices[device] = pNeoDeviceEx;

	return true;
}

int LegacyDLLExport icsneoClosePort(void* hObject, int* pNumberOfErrors)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	if (pNumberOfErrors) {
		*pNumberOfErrors = 0;
	}
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (openneodevices.find(device) != openneodevices.end()) {
		openneodevices[device]->neoDevice.NumberOfClients -= 1;
		openneodevices.erase(device);
	}
	return icsneo_closeDevice(device);
}

// Memory is now managed automatically, this function is unneeded
void LegacyDLLExport icsneoFreeObject(void* hObject)
{
	(void)hObject;
	return;
}

int LegacyDLLExport icsneoSerialNumberToString(unsigned long serial, char* data, unsigned long data_size)
{
	size_t length = (size_t)data_size;
	return icsneo_serialNumToString((uint32_t)serial, data, &length);
}

//Message Functions
int LegacyDLLExport icsneoGetMessages(void* hObject, icsSpyMessage* pMsg, int* pNumberOfMessages, int* pNumberOfErrors)
{
	static neomessage_t messages[20000];
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	size_t messageCount = 20000;
	if (!icsneo_getMessages(device, messages, &messageCount, 0))
		return false;

	*pNumberOfMessages = 0;
	*pNumberOfErrors = 0;

	for (size_t i = 0; i < messageCount; i++)
	{
		if (NeoMessageToSpyMessage(device, messages[i], pMsg[*pNumberOfMessages]))
			(*pNumberOfMessages)++;
	}

	return true;
}

int LegacyDLLExport icsneoTxMessages(void* hObject, icsSpyMessage* pMsg, int lNetworkID, int lNumMessages)
{
	return icsneoTxMessagesEx(hObject, pMsg, lNetworkID, lNumMessages, nullptr, 0);
}

int LegacyDLLExport icsneoTxMessagesEx(void* hObject, icsSpyMessage* pMsg, unsigned int lNetworkID, unsigned int lNumMessages,
	unsigned int* NumTxed, unsigned int zero2)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	neomessage_frame_t newmsg;
	unsigned int temp = 0;
	if (NumTxed == nullptr)
		NumTxed = &temp;
	*NumTxed = 0;
	for (unsigned int i = 0; i < lNumMessages; i++)
	{
		newmsg = {};
		const icsSpyMessage& oldMsg = pMsg[i];
		SpyMessageToNeoMessage(oldMsg, newmsg, lNetworkID);
		if (icsneo_transmit(device, reinterpret_cast<neomessage_t*>(&newmsg)))
			(*NumTxed)++;
	}
	return lNumMessages == *NumTxed;
}

int LegacyDLLExport icsneoEnableNetworkRXQueue(void* hObject, int iEnable)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetTimeStampForMsg(void* hObject, icsSpyMessage* pMsg, double* pTimeStamp) {
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

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

void LegacyDLLExport icsneoGetISO15765Status(void* hObject, int lNetwork, int lClearTxStatus, int lClearRxStatus,
	int* lTxStatus, int* lRxStatus)
{
	// TODO Implement
	return;
}

void LegacyDLLExport icsneoSetISO15765RxParameters(void* hObject, int lNetwork, int lEnable, spyFilterLong* pFF_CFMsgFilter,
	icsSpyMessage* pTxMsg, int lCFTimeOutMs, int lFlowCBlockSize, int lUsesExtendedAddressing, int lUseHardwareIfPresent)
{
	// TODO Implement
	return;
}

int LegacyDLLExport icsneoGetRTC(void* hObject, icsSpyTime* time)
{
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	uint64_t time64 = 0;
	if(!icsneo_getRTC(device, &time64))
		return false;

	std::time_t seconds = time64;
	/* To accommodate local time bugzilla #6600 https://intrepidcs.homeip.net:100/bugzilla/show_bug.cgi?id=6600 */
	// local time must be used here
	const auto timeInfo = std::localtime(&seconds);
	if(!timeInfo)
		return false;

	time->sec = (unsigned char)timeInfo->tm_sec; // Will never hit 60 (leap second) because tm_sec comes from RTCCTIME
	time->min = (unsigned char)timeInfo->tm_min;
	time->hour = (unsigned char)timeInfo->tm_hour;
	time->day = (unsigned char)timeInfo->tm_mday;
	time->month = (unsigned char)timeInfo->tm_mon + 1;
	time->year = (unsigned char)timeInfo->tm_year % 100;

	return true;
}

int LegacyDLLExport icsneoSetRTC(void* hObject, const icsSpyTime* time)
{
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	std::tm timeInfo{};
	timeInfo.tm_sec = time->sec;
	timeInfo.tm_min = time->min;
	timeInfo.tm_hour = time->hour;
	timeInfo.tm_mday = time->day;
	timeInfo.tm_mon = time->month - 1;
	timeInfo.tm_year = time->year + 100;

	#ifdef _MSC_VER
		#define timegm _mkgmtime
	#endif

	return icsneo_setRTC(device, (uint64_t)timegm(&timeInfo));
}

//Device Functions
int LegacyDLLExport icsneoGetConfiguration(void* hObject, unsigned char* pData, int* lNumBytes)
{
	// 2G devices are not supported in the new API
	return false;
}

int LegacyDLLExport icsneoSendConfiguration(void* hObject, unsigned char* pData, int lNumBytes)
{
	// 2G devices are not supported in the new API
	return false;
}

int LegacyDLLExport icsneoGetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetFireSettings(void* hObject, SFireSettings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCAN3Settings(void* hObject, SVCAN3Settings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetFire2Settings(void* hObject, SFire2Settings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCANRFSettings(void* hObject, SVCANRFSettings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetVCAN412Settings(void* hObject, SVCAN412Settings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetRADGalaxySettings(void* hObject, SRADGalaxySettings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!(icsneo_settingsReadStructure(device, pSettings, iNumBytes) + 1);
}

int LegacyDLLExport icsneoSetRADStar2Settings(void* hObject, SRADStar2Settings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoSetBitRate(void* hObject, int BitRate, int NetworkID)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (!icsneo_setBaudrate(device, (uint16_t)NetworkID, BitRate))
		return false;
	return icsneo_settingsApply(device);
}

int LegacyDLLExport icsneoSetFDBitRate(void* hObject, int BitRate, int NetworkID)
{
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if(!icsneo_setFDBaudrate(device, (uint16_t)NetworkID, BitRate))
		return false;
	return icsneo_settingsApply(device);
}

int LegacyDLLExport icsneoGetDeviceParameters(void* hObject, char* pParameter, char* pValues, short ValuesLength)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoSetDeviceParameters(void* hObject, char* pParmValue, int* pErrorIndex, int bSaveToEEPROM)
{
	// TODO Implement
	return false;
}

// Error Functions
int LegacyDLLExport icsneoGetLastAPIError(void* hObject, unsigned long* pErrorNumber)
{
	if(!icsneoValidateHObject(hObject))
		return false;

	neoevent_t error;
	icsneo_getLastError(&error);
	*pErrorNumber = (unsigned long)error.eventNumber;
	return true;
}

int LegacyDLLExport icsneoGetErrorMessages(void* hObject, int* pErrorMsgs, int* pNumberOfErrors)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetErrorInfo(int lErrorNumber, char* szErrorDescriptionShort, char* szErrorDescriptionLong,
	int* lMaxLengthShort, int* lMaxLengthLong, int* lErrorSeverity, int* lRestartNeeded)
{
	if (szErrorDescriptionShort == nullptr || szErrorDescriptionLong == nullptr 
		|| lMaxLengthShort == nullptr || lMaxLengthLong == nullptr || lErrorSeverity == nullptr 
		|| lRestartNeeded == nullptr)
	{
		return false;
	}
	
	//Set and send back 0. We will not restart the software.
	*lRestartNeeded = 0;
	
	//Using the error number, get the description from the event.
	const char* tempDescription = APIEvent::DescriptionForType(APIEvent::Type(lErrorNumber));
	int descrLength = int(std::strlen(tempDescription));
	
	//Check to make sure the length of the error is not >= the buffer.
	if (descrLength >= *lMaxLengthShort || descrLength >= *lMaxLengthLong)
	{
		return false;
	}

	//Copy the error description to the inout Short and Long arguments.
	std::copy(tempDescription, tempDescription + descrLength, szErrorDescriptionShort);
	std::copy(tempDescription, tempDescription + descrLength, szErrorDescriptionLong);

	//Add the null terminator.
	szErrorDescriptionShort[descrLength] = '\0';
	szErrorDescriptionLong[descrLength] = '\0';

	//Update the inout lengths to what the actual length of the error is
	*lMaxLengthShort = *lMaxLengthLong = descrLength;
	
	//Update the inout severity argument.
	*lErrorSeverity = int(APIEvent::Severity::Any);
	return true;
}

//ISO15765-2 Functions
int LegacyDLLExport icsneoISO15765_EnableNetworks(void* hObject, unsigned long ulNetworks)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_DisableNetworks(void* hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_TransmitMessage(void* hObject, unsigned long ulNetworkID, stCM_ISO157652_TxMessage* pMsg,
	unsigned long ulBlockingTimeout)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoISO15765_ReceiveMessage(void* hObject, int ulNetworkID, stCM_ISO157652_RxMessage* pMsg)
{
	// TODO Implement
	return false;
}

//General Utility Functions
int LegacyDLLExport icsneoValidateHObject(void* hObject)
{
	for (auto it = neodevices.begin(); it != neodevices.end(); it++)
	{
		if (&it->second == hObject)
		{
			neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
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

int LegacyDLLExport icsneoGetSerialNumber(void* hObject, unsigned int* iSerialNumber)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	*iSerialNumber = icsneo_serialStringToNum(device->serial);
	return true;
}

int LegacyDLLExport icsneoEnableDOIPLine(void* hObject, bool enable)
{
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return icsneo_setDigitalIO(device, ICSNEO_IO_ETH_ACTIVATION, 1, enable);
}

int LegacyDLLExport icsneoStartSockServer(void* hObject, int iPort)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoStopSockServer(void* hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoGetDeviceStatus(void* hObject, icsDeviceStatus* deviceStatus, size_t* deviceStatusSize)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	if (deviceStatus == nullptr || deviceStatusSize == nullptr)
		return false;

	return icsneo_getDeviceStatus(device, deviceStatus, deviceStatusSize);
}

//CoreMini Script functions
int LegacyDLLExport icsneoScriptStart(void* hObject, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStop(void* hObject)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptLoad(void* hObject, const unsigned char* bin, unsigned long len_bytes, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptClear(void* hObject, int iLocation)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStartFBlock(void* hObject, unsigned int fb_index)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptGetFBlockStatus(void* hObject, unsigned int fb_index, int* piRunStatus)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptStopFBlock(void* hObject, unsigned int fb_index)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptGetScriptStatus(void* hObject, int* piStatus)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptReadAppSignal(void* hObject, unsigned int iIndex, double* dValue)
{
	// TODO Implement
	return false;
}

int LegacyDLLExport icsneoScriptWriteAppSignal(void* hObject, unsigned int iIndex, double dValue)
{
	// TODO Implement
	return false;
}



int LegacyDLLExport icsneoEnableNetworkCom(void* hObject, int Enable)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	if (Enable)
		return icsneo_goOnline(device);
	else
		return icsneo_goOffline(device);
}


int LegacyDLLExport icsneoGetVCAN4Settings(void* hObject, SVCAN4Settings* pSettings, int iNumBytes)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return !!icsneo_settingsReadStructure(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoSetVCAN4Settings(void* hObject, SVCAN4Settings* pSettings, int iNumBytes, int bSaveToEEPROM)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (bSaveToEEPROM)
		return icsneo_settingsApplyStructure(device, pSettings, iNumBytes);
	return icsneo_settingsApplyStructureTemporary(device, pSettings, iNumBytes);
}

int LegacyDLLExport icsneoGetDeviceSettingsType(void* hObject, EPlasmaIonVnetChannel_t vnetSlot, EDeviceSettingsType* pDeviceSettingsType)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

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
	case NEODEVICE_RADMOON3:
		*pDeviceSettingsType = DeviceRADMoon3SettingsType;
		break;
	case NEODEVICE_RADGEMINI:
		*pDeviceSettingsType = DeviceRADGeminiSettingsType;
		break;
	case NEODEVICE_RED2:
		*pDeviceSettingsType = DeviceRed2SettingsType;
		break;
	case NEODEVICE_FIRE3:
		*pDeviceSettingsType = DeviceFire3SettingsType;
		break;
	case NEODEVICE_FIRE3_FLEXRAY:
		*pDeviceSettingsType = DeviceFire3FlexraySettingsType;
		break;
	default:
		return 0;
	}

	return 1;
}

int LegacyDLLExport icsneoSetDeviceSettings(void* hObject, SDeviceSettings* pSettings, int iNumBytes, int bSaveToEEPROM,
	EPlasmaIonVnetChannel_t vnetSlot)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

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

	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

	if (icsneoGetDeviceSettingsType(hObject, vnetSlot, &pSettings->DeviceSettingType) == 0)
		return false;

	const size_t offset = size_t(&pSettings->Settings) - size_t(pSettings);
	return !!icsneo_settingsReadStructure(device, &pSettings->Settings, iNumBytes - offset);
}

int LegacyDLLExport icsneoLoadDefaultSettings(void* hObject)
{
	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	return icsneo_settingsApplyDefaults(device);
}

int LegacyDLLExport icsneoSetBitRateEx(void* hObject, unsigned long BitRate, int NetworkID, int iOptions)
{
	if (!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);
	if (!icsneo_setBaudrate(device, (uint16_t)NetworkID, BitRate))
		return false;

	bool bSaveToEEPROM = iOptions & 0x1;

	if (bSaveToEEPROM)
		return icsneo_settingsApply(device);
	else
		return icsneo_settingsApplyTemporary(device);
}

int LegacyDLLExport icsneoEnableNetworkComEx(void* hObject, int iEnable, int iNetId)
{
	return icsneoEnableNetworkCom(hObject, iEnable);
}

int LegacyDLLExport icsneoForceFirmwareUpdate(void* hObject)
{
	return false;
}

int LegacyDLLExport icsneoGetHWFirmwareInfo(void* hObject, stAPIFirmwareInfo *pInfo)
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

int LegacyDLLExport icsneoEnableBitSmash(void* hObject,
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

int LegacyDLLExport icsneoDisableBitSmash(void* hObject, unsigned int reservedZero)
{
	return false;
}

int LegacyDLLExport icsneoSendHWTimeRequest(void* hObject)
{
	return false;
}

int LegacyDLLExport icsneoReceiveHWTimeResponse(void* hObject, double* TimeHardware, unsigned long TimeOut)
{
	return false;
}

int LegacyDLLExport icsneoGetDLLFirmwareInfo(void* hObject, stAPIFirmwareInfo* pInfo)
{
	return false;
}

int LegacyDLLExport icsneoGetDLLFirmwareInfoEx(void* hObject, stAPIFirmwareInfo* pInfo, EPlasmaIonVnetChannel_t vnetSlot)
{
	return false;
}

int LegacyDLLExport icsneoJ2534Cmd(void* hObject, unsigned char* CmdBuf, short Len, void* pVoid)
{
	uint64_t* pTmp = nullptr;
	int iRetVal = 0, iNumBytes = 0;
	uint16_t NetworkID = 0;

	if (!icsneoValidateHObject(hObject))
		return false;

	neodevice_t* device = reinterpret_cast<neodevice_t*>(hObject);

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
			if (icsneoGetFire2Settings(hObject, &Cs, iNumBytes))
			{
				const auto cmId = mp_HWnetIDToCMnetID.find(GetVnetAgnosticNetid(NetworkID));
				if (cmId != mp_HWnetIDToCMnetID.end())
				{
					if (Cs.termination_enables & (1ull << cmId->second))
						*pTmp = 3; // Termination ON
					else
						*pTmp = 0; // Termination OFF
				}
				else
				{
					iRetVal = 0;
				}
			}
			else
			{
				iRetVal = 0;
			}
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
				const auto cmId = mp_HWnetIDToCMnetID.find(GetVnetAgnosticNetid(NetworkID));
				if (cmId != mp_HWnetIDToCMnetID.end())
				{
					if (*pTmp == 3) /*Termination ON*/
						Cs.termination_enables |= (1ull << cmId->second);
					else /*Termination OFF*/
						Cs.termination_enables &= ~(1ull << cmId->second);

					iRetVal = icsneoSetFire2Settings(hObject, &Cs, iNumBytes, 1 /* ConfigurationOptionDoNotSaveToEEPROM */);
				}
				else
				{
					iRetVal = 0;
				}
			}
			else
			{
				iRetVal = 0;
			}
			break;
		}
		break;

	default:
		break;
	}
	return iRetVal;
}

int LegacyDLLExport icsneoSendRawCmds(void* hObject, icsneoVICommand* pCmdMsgs, int lNumOfCmds)
{
	return false;
}

int LegacyDLLExport icsneoEnableBusVoltageMonitor(void* hObject, unsigned int enable, unsigned int reserved)
{
	return false;
}

int LegacyDLLExport icsneoISO15765_TransmitMessageEx(void* hObject,
	unsigned long ulNetworkID,
	ISO15765_2015_TxMessage* pMsg,
	unsigned long ulBlockingTimeout)
{
	return false;
}

int LegacyDLLExport icsneoGetBusVoltage(void* hObject, unsigned long* pVBusVoltage, unsigned int reserved)
{
	return false;
}

int LegacyDLLExport icsneoOpenRemoteNeoDevice(const char* pIPAddress, NeoDevice* pNeoDevice, void** hObject,
	unsigned char* bNetworkIDs, int iOptions)
{
	return false;
}

int LegacyDLLExport icsneoFindRemoteNeoDevices(const char* pIPAddress, NeoDevice* pNeoDevice, int* pNumDevices)
{
	return false;
}

int LegacyDLLExport icsneoFirmwareUpdateRequired(void* hObject)
{
	return false;
}

void LegacyDLLExport icsneoGetDLLVersionEx(unsigned long* dwMSVersion, unsigned long* dwLSVersion)
{
	return;
}

int LegacyDLLExport icsneoGetNetidforSlaveVNETs(size_t* NetworkIndex, EPlasmaIonVnetChannel_t vnetSlot)
{
	return GetVnetNetid(*NetworkIndex, vnetSlot);
}

int LegacyDLLExport icsneoGetVnetSimpleNetid(size_t* FullNetID)
{
	*FullNetID = GetVnetAgnosticNetid(*FullNetID);
	return true;
}

int LegacyDLLExport icsneoSerialNumberFromString(unsigned long* serial, char* data)
{
	if (serial == nullptr)
		return false;

	*serial = icsneo_serialStringToNum(data);
	return false;
}

int LegacyDLLExport icsneoGetMiniportAdapterInfo(void* hObject, NDIS_ADAPTER_INFORMATION* aInfo)
{
	return false;
}
