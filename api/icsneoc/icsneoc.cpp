#ifndef __cplusplus
#error "icsneoc.cpp must be compiled with a C++ compiler!"
#endif

#define ICSNEOC_MAKEDLL

#include "include/icsneoc.h"
#include "api/icsneocpp/include/icsneocpp.h"
#include "platform/include/dynamiclib.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <map>

using namespace icsneo;

// Holds references for the shared_ptrs so they do not get freed until we're ready
static std::vector<std::shared_ptr<Device>> connectableFoundDevices, connectedDevices;

// We store an array of shared_ptr messages per device, this is the owner of the shared_ptr on behalf of the C interface
static std::map<devicehandle_t, std::vector<std::shared_ptr<Message>>> polledMessageStorage;

void icsneoFindAllDevices(neodevice_t* devices, size_t* count) {
	icsneoFreeUnconnectedDevices(); // Mark previous results as freed so they can no longer be connected to
	auto foundDevices = icsneo::FindAllDevices();

	auto inputSize = *count;
	*count = foundDevices.size();
	auto outputSize = *count;
	if(outputSize > inputSize) {
		// TODO an error should be returned that the data was truncated
		outputSize = inputSize;
	}

	for(size_t i = 0; i < outputSize; i++) {
		connectableFoundDevices.push_back(foundDevices[i]);
		devices[i] = foundDevices[i]->getNeoDevice();
	}
}

void icsneoFreeUnconnectedDevices() {
	connectableFoundDevices.clear();
}

bool icsneoSerialNumToString(uint32_t num, char* str, size_t* count) {
	auto result = Device::SerialNumToString(num);
	if(*count <= result.length()) {
		*count = result.length() + 1; // This is how big of a buffer we need
		return false;
	}
	strncpy(str, result.c_str(), *count);
	str[*count - 1] = '\0';
	*count = result.length();
	return true;
}

uint32_t icsneoSerialStringToNum(const char* str) {
	return Device::SerialStringToNum(str);
}

bool icsneoIsValidNeoDevice(const neodevice_t* device) {
	// If this neodevice_t was returned by a previous search, it will no longer be valid (as the underlying icsneo::Device is freed)
	for(auto& dev : connectedDevices) {
		if(dev.get() == device->device)
			return true;
	}
	for(auto& dev : connectableFoundDevices) {
		if(dev.get() == device->device)
			return true;
	}
	return false;
}

bool icsneoOpenDevice(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	if(!device->device->open())
		return false;

	// We connected successfully, move the device to connected devices
	std::vector<std::vector<std::shared_ptr<Device>>::iterator> itemsToMove;
	for(auto it = connectableFoundDevices.begin(); it < connectableFoundDevices.end(); it++) {
		if((*it).get() == device->device)
			itemsToMove.push_back(it);
	}
	for(auto it : itemsToMove) {
		connectedDevices.push_back(*it);
		connectableFoundDevices.erase(it);
	}

	return true;
}

bool icsneoCloseDevice(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;
	
	if(!device->device->close())
		return false;

	// We disconnected successfully, free the device
	std::vector<std::vector<std::shared_ptr<Device>>::iterator> itemsToDelete;
	for(auto it = connectedDevices.begin(); it < connectedDevices.end(); it++) {
		if((*it).get() == device->device)
			itemsToDelete.push_back(it);
	}
	for(auto it : itemsToDelete)
		connectedDevices.erase(it);

	return true;
}

bool icsneoGoOnline(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	return device->device->goOnline();
}

bool icsneoGoOffline(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	return device->device->goOffline();
}

bool icsneoIsOnline(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	return device->device->isOnline();
}

bool icsneoEnableMessagePolling(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	device->device->enableMessagePolling();
	return true;
}

bool icsneoDisableMessagePolling(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	return device->device->disableMessagePolling();
}

bool icsneoGetMessages(const neodevice_t* device, neomessage_t* messages, size_t* items) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	if(items == nullptr)
		return false;

	if(messages == nullptr) {
		// A NULL value for messages means the user wants the current size of the buffer into items
		*items = device->device->getCurrentMessageCount();
		return true;
	}

	std::vector<std::shared_ptr<Message>>& storage = polledMessageStorage[device->device];

	if(!device->device->getMessages(storage, *items))
		return false;

	*items = storage.size();

	for(size_t i = 0; i < *items; i++) {
		// For each message, copy into neomessage_t buffer given
		messages[i] = CreateNeoMessage(*(storage[i]));
	}

	// The user now has until the next call of icsneoGetMessages (for this device) to use the data, after which point it's freed
	// The user should copy the data out if they want it
	return true;
}

size_t icsneoGetPollingMessageLimit(const neodevice_t* device) {
	if(!icsneoIsValidNeoDevice(device))
		return 0;

	return device->device->getPollingMessageLimit();
}

bool icsneoSetPollingMessageLimit(const neodevice_t* device, size_t newLimit) {
	if(!icsneoIsValidNeoDevice(device))
		return false;

	device->device->setPollingMessageLimit(newLimit);
	return true;
}