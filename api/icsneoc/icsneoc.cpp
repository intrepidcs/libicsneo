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

using namespace icsneo;

// Holds references for the shared_ptrs so they do not get freed until we're ready
static std::vector<std::shared_ptr<Device>> connectableFoundDevices, connectedDevices;

// Any shared_ptrs we've let go should be placed here so they're not accessed
static std::vector<Device*> freedDevices;

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
	for(auto& devptr : connectableFoundDevices) {
		freedDevices.push_back(devptr.get());
	}
	connectableFoundDevices.clear();
}

bool icsneoSerialNumToString(uint32_t num, char* str, size_t* count) {
	auto result = Device::SerialNumToString(num);
	if(*count <= result.length()) {
		*count = result.length() + 1; // This is how big of a buffer we need
		return false;
	}
	strcpy(str, result.c_str()); // TODO bad
	*count = result.length();
	return true;
}

uint32_t icsneoSerialStringToNum(const char* str) {
	return Device::SerialStringToNum(str);
}

bool icsneoIsValidNeoDevice(const neodevice_t* device) {
	// If this neodevice_t was returned by a previous search, it will no longer be valid (as the underlying icsneo::Device is freed)
	return std::find(freedDevices.begin(), freedDevices.end(), device->device) == freedDevices.end();
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

	// We disconnected successfully, free the device and mark it as freed
	std::vector<std::vector<std::shared_ptr<Device>>::iterator> itemsToDelete;
	for(auto it = connectedDevices.begin(); it < connectedDevices.end(); it++) {
		if((*it).get() == device->device)
			itemsToDelete.push_back(it);
	}
	for(auto it : itemsToDelete)
		connectedDevices.erase(it);
	
	freedDevices.push_back(device->device);

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