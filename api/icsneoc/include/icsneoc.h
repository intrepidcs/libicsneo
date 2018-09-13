#ifndef __ICSNEOC_H_
#define __ICSNEOC_H_

#include <stddef.h>
#include "device/include/neodevice.h" // For neodevice_t
#include "communication/message/include/neomessage.h" // For neomessage_t and friends
#include "platform/include/dynamiclib.h" // Dynamic library loading and exporting

#ifndef ICSNEOC_DYNAMICLOAD

#ifdef __cplusplus
extern "C" {
#endif

extern void DLLExport icsneoFindAllDevices(neodevice_t* devices, size_t* count);

extern void DLLExport icsneoFreeUnconnectedDevices();

extern bool DLLExport icsneoSerialNumToString(uint32_t num, char* str, size_t* count);

extern uint32_t DLLExport icsneoSerialStringToNum(const char* str);

extern bool DLLExport icsneoIsValidNeoDevice(const neodevice_t* device);

extern bool DLLExport icsneoOpenDevice(const neodevice_t* device);

extern bool DLLExport icsneoCloseDevice(const neodevice_t* device);

extern bool DLLExport icsneoGoOnline(const neodevice_t* device);

extern bool DLLExport icsneoGoOffline(const neodevice_t* device);

extern bool DLLExport icsneoIsOnline(const neodevice_t* device);

extern bool DLLExport icsneoEnableMessagePolling(const neodevice_t* device);

extern bool DLLExport icsneoDisableMessagePolling(const neodevice_t* device);

extern bool DLLExport icsneoGetMessages(const neodevice_t* device, neomessage_t* messages, size_t* items);

extern size_t DLLExport icsneoGetPollingMessageLimit(const neodevice_t* device);

extern bool DLLExport icsneoSetPollingMessageLimit(const neodevice_t* device, size_t newLimit);

#ifdef __cplusplus
}
#endif

#else // ICSNEOC_DYNAMICLOAD

typedef void(*fn_icsneoFindAllDevices)(neodevice_t* devices, size_t* count);
fn_icsneoFindAllDevices icsneoFindAllDevices;

typedef void(*fn_icsneoFreeUnconnectedDevices)();
fn_icsneoFreeUnconnectedDevices icsneoFreeUnconnectedDevices;

typedef bool(*fn_icsneoSerialNumToString)(uint32_t num, char* str, size_t* count);
fn_icsneoSerialNumToString icsneoSerialNumToString;

typedef uint32_t(*fn_icsneoSerialStringToNum)(const char* str);
fn_icsneoSerialStringToNum icsneoSerialStringToNum;

typedef bool(*fn_icsneoIsValidNeoDevice)(const neodevice_t* device);
fn_icsneoIsValidNeoDevice icsneoIsValidNeoDevice;

typedef bool(*fn_icsneoOpenDevice)(const neodevice_t* device);
fn_icsneoOpenDevice icsneoOpenDevice;

typedef bool(*fn_icsneoCloseDevice)(const neodevice_t* device);
fn_icsneoCloseDevice icsneoCloseDevice;

typedef bool(*fn_icsneoGoOnline)(const neodevice_t* device);
fn_icsneoGoOnline icsneoGoOnline;

typedef bool(*fn_icsneoGoOffline)(const neodevice_t* device);
fn_icsneoGoOffline icsneoGoOffline;

typedef bool(*fn_icsneoIsOnline)(const neodevice_t* device);
fn_icsneoIsOnline icsneoIsOnline;

typedef bool(*fn_icsneoEnableMessagePolling)(const neodevice_t* device);
fn_icsneoEnableMessagePolling icsneoEnableMessagePolling;

typedef bool(*fn_icsneoDisableMessagePolling)(const neodevice_t* device);
fn_icsneoDisableMessagePolling icsneoDisableMessagePolling;

typedef bool(*fn_icsneoGetMessages)(const neodevice_t* device, neomessage_t* messages, size_t* items);
fn_icsneoGetMessages icsneoGetMessages;

typedef size_t(*fn_icsneoGetPollingMessageLimit)(const neodevice_t* device);
fn_icsneoGetPollingMessageLimit icsneoGetPollingMessageLimit;

typedef bool(*fn_icsneoSetPollingMessageLimit)(const neodevice_t* device, size_t newLimit);
fn_icsneoSetPollingMessageLimit icsneoSetPollingMessageLimit;

#define ICSNEO_IMPORT(func) func = (fn_##func)icsneoDynamicLibraryGetFunction(icsneoLibraryHandle, #func)
#define ICSNEO_IMPORTASSERT(func) if((ICSNEO_IMPORT(func)) == NULL) return 3
void* icsneoLibraryHandle = NULL;
bool icsneoInitialized = false;
bool icsneoDestroyed = false;
int icsneoInit() {
	icsneoDestroyed = false;
	if(icsneoInitialized)
		return 1;

	icsneoLibraryHandle = icsneoDynamicLibraryLoad();
	if(icsneoLibraryHandle == NULL)
		return 2;

	ICSNEO_IMPORTASSERT(icsneoFindAllDevices);
	ICSNEO_IMPORTASSERT(icsneoFreeUnconnectedDevices);
	ICSNEO_IMPORTASSERT(icsneoSerialNumToString);
	ICSNEO_IMPORTASSERT(icsneoSerialStringToNum);
	ICSNEO_IMPORTASSERT(icsneoIsValidNeoDevice);
	ICSNEO_IMPORTASSERT(icsneoOpenDevice);
	ICSNEO_IMPORTASSERT(icsneoCloseDevice);
	ICSNEO_IMPORTASSERT(icsneoGoOnline);
	ICSNEO_IMPORTASSERT(icsneoGoOffline);
	ICSNEO_IMPORTASSERT(icsneoIsOnline);
	ICSNEO_IMPORTASSERT(icsneoEnableMessagePolling);
	ICSNEO_IMPORTASSERT(icsneoDisableMessagePolling);
	ICSNEO_IMPORTASSERT(icsneoGetMessages);
	ICSNEO_IMPORTASSERT(icsneoGetPollingMessageLimit);
	ICSNEO_IMPORTASSERT(icsneoSetPollingMessageLimit);

	icsneoInitialized = true;
	return 0;
}

bool icsneoClose() ICSNEO_DESTRUCTOR {
	icsneoInitialized = false;
	if(icsneoDestroyed)
		return true;

	return icsneoDestroyed = icsneoDynamicLibraryClose(icsneoLibraryHandle);
}

#endif // ICSNEOC_DYNAMICLOAD

#endif // __ICSNEOC_H_