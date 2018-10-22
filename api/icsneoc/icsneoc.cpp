#ifndef __cplusplus
#error "icsneoc.cpp must be compiled with a C++ compiler!"
#endif

#define ICSNEOC_MAKEDLL

#include "icsneo/icsneoc.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/platform/dynamiclib.h"
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

void icsneo_findAllDevices(neodevice_t* devices, size_t* count) {
	std::vector<std::shared_ptr<Device>> foundDevices = icsneo::FindAllDevices();
	
	if(count == nullptr)
		return;

	if(devices == nullptr) {
		*count = foundDevices.size();
		return;
	}

	icsneo_freeUnconnectedDevices(); // Mark previous results as freed so they can no longer be connected to

	size_t inputSize = *count;
	*count = foundDevices.size();
	size_t outputSize = *count;
	if(outputSize > inputSize) {
		// TODO an error should be returned that the data was truncated
		outputSize = inputSize;
	}

	for(size_t i = 0; i < outputSize; i++) {
		connectableFoundDevices.push_back(foundDevices[i]);
		devices[i] = foundDevices[i]->getNeoDevice();
	}
}

void icsneo_freeUnconnectedDevices() {
	connectableFoundDevices.clear();
}

bool icsneo_serialNumToString(uint32_t num, char* str, size_t* count) {
	auto result = Device::SerialNumToString(num);
	if(*count < result.length()) {
		*count = result.length() + 1; // This is how big of a buffer we need
		return false;
	}

	*count = result.copy(str, *count);
	str[*count] = '\0';
	return true;
}

uint32_t icsneo_serialStringToNum(const char* str) {
	return Device::SerialStringToNum(str);
}

bool icsneo_isValidNeoDevice(const neodevice_t* device) {
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

bool icsneo_openDevice(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
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

bool icsneo_closeDevice(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
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

bool icsneo_goOnline(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->goOnline();
}

bool icsneo_goOffline(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->goOffline();
}

bool icsneo_isOnline(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->isOnline();
}

bool icsneo_enableMessagePolling(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	device->device->enableMessagePolling();
	return true;
}

bool icsneo_disableMessagePolling(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->disableMessagePolling();
}

bool icsneo_getMessages(const neodevice_t* device, neomessage_t* messages, size_t* items) {
	if(!icsneo_isValidNeoDevice(device))
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
		messages[i] = CreateNeoMessage(storage[i]);
	}

	// The user now has until the next call of icsneo_getMessages (for this device) to use the data, after which point it's freed
	// The user should copy the data out if they want it
	return true;
}

size_t icsneo_getPollingMessageLimit(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return 0;

	return device->device->getPollingMessageLimit();
}

bool icsneo_setPollingMessageLimit(const neodevice_t* device, size_t newLimit) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	device->device->setPollingMessageLimit(newLimit);
	return true;
}

bool icsneo_getProductName(const neodevice_t* device, char* str, size_t* maxLength) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	*maxLength = device->device->getType().toString().copy(str, *maxLength);
	str[*maxLength] = '\0';
	return true;
}

bool icsneo_settingsRefresh(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->refresh();
}

bool icsneo_settingsApply(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->apply();
}

bool icsneo_settingsApplyTemporary(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->apply(true);
}

bool icsneo_settingsApplyDefaults(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->applyDefaults();
}

bool icsneo_settingsApplyDefaultsTemporary(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->applyDefaults(true);
}

bool icsneo_setBaudrate(const neodevice_t* device, uint16_t netid, uint32_t newBaudrate) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->setBaudrateFor(netid, newBaudrate);
}

bool icsneo_transmit(const neodevice_t* device, const neomessage_t* message) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->transmit(CreateMessageFromNeoMessage(message));
}

bool icsneo_transmitMessages(const neodevice_t* device, const neomessage_t* messages, size_t count) {
	// TODO This can be implemented faster
	for(size_t i = 0; i < count; i++) {
		if(!icsneo_transmit(device, messages + i))
			return false;
	}
	return true;
}