#ifndef __cplusplus
#error "icsneoc.cpp must be compiled with a C++ compiler!"
#endif

#define ICSNEOC_MAKEDLL

#include "icsneo/icsneoc.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/platform/dynamiclib.h"
#include "icsneo/api/errormanager.h"
#include "icsneo/device/devicefinder.h"
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
	
	if(count == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return;
	}

	if(devices == nullptr) {
		*count = foundDevices.size();
		return;
	}

	icsneo_freeUnconnectedDevices(); // Mark previous results as freed so they can no longer be connected to

	size_t inputSize = *count;
	*count = foundDevices.size();
	size_t outputSize = *count;
	if(outputSize > inputSize) {
		ErrorManager::GetInstance().add(APIError::OutputTruncated);
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
	// TAG String copy function
	if(count == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	auto result = Device::SerialNumToString(num);

	if(str == nullptr) {
		*count = result.length() + 1;
		return false;
	}

	if(*count < result.length()) {
		*count = result.length() + 1; // This is how big of a buffer we need
		ErrorManager::GetInstance().add(APIError::BufferInsufficient);
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
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

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
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}
	
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
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->goOnline();
}

bool icsneo_goOffline(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->goOffline();
}

bool icsneo_isOnline(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->isOnline();
}

bool icsneo_enableMessagePolling(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	device->device->enableMessagePolling();
	return true;
}

bool icsneo_disableMessagePolling(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->disableMessagePolling();
}

bool icsneo_getMessages(const neodevice_t* device, neomessage_t* messages, size_t* items, uint64_t timeout) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	if(items == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	if(messages == nullptr) {
		// A NULL value for messages means the user wants the current size of the buffer into items
		*items = device->device->getCurrentMessageCount();
		return true;
	}

	std::vector<std::shared_ptr<Message>>& storage = polledMessageStorage[device->device];

	if(!device->device->getMessages(storage, *items, std::chrono::milliseconds(timeout)))
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
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return 0;
	}

	return device->device->getPollingMessageLimit();
}

bool icsneo_setPollingMessageLimit(const neodevice_t* device, size_t newLimit) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	device->device->setPollingMessageLimit(newLimit);
	return true;
}

bool icsneo_getProductName(const neodevice_t* device, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	std::string output = device->device->getType().toString();

	if(str == nullptr) {
		*maxLength = output.length();
		return false;
	}

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		ErrorManager::GetInstance().add(APIError::OutputTruncated);

	return true;
}

bool icsneo_getProductNameForType(devicetype_t type, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	std::string output = DeviceType(type).toString();

	if(str == nullptr) {
		*maxLength = output.length();
		return false;
	}

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		ErrorManager::GetInstance().add(APIError::OutputTruncated);

	return true;
}

bool icsneo_settingsRefresh(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->refresh();
}

bool icsneo_settingsApply(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->apply();
}

bool icsneo_settingsApplyTemporary(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->apply(true);
}

bool icsneo_settingsApplyDefaults(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->applyDefaults();
}

bool icsneo_settingsApplyDefaultsTemporary(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->applyDefaults(true);
}

size_t icsneo_settingsReadStructure(const neodevice_t* device, void* structure, size_t structureSize) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return 0;
	}

	size_t readSize = device->device->settings->getSize();
	if(structure == nullptr) // Structure size request
		return readSize;
	if(readSize > structureSize) {
		// Client application has a smaller structure than we do
		// It is probably built against an older version of the API
		ErrorManager::GetInstance().add(APIError::OutputTruncated);
		readSize = structureSize;
	}

	const void* deviceStructure = device->device->settings->getRawStructurePointer();
	if(deviceStructure == nullptr) {
		ErrorManager::GetInstance().add(APIError::SettingsNotAvailable);
		return 0;
	}

	memcpy(structure, deviceStructure, readSize);
	
	if(readSize < structureSize) // Client application is attempting to read more than we have
		memset((uint8_t*)structure + readSize, 0, structureSize - readSize);

	return readSize;
}

// Not exported
static bool icsneo_settingsWriteStructure(const neodevice_t* device, const void* structure, size_t structureSize) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	if(structure == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	size_t writeSize = device->device->settings->getSize();
	if(writeSize < structureSize) {
		ErrorManager::GetInstance().add(APIError::OutputTruncated);
		structureSize = writeSize;
	}

	void* deviceStructure = device->device->settings->getMutableRawStructurePointer();
	if(deviceStructure == nullptr) {
		ErrorManager::GetInstance().add(APIError::SettingsNotAvailable);
		return false;
	}

	memcpy(deviceStructure, structure, structureSize);

	// If writeSize > structureSize that means that the user has given us a smaller structure
	// This is okay, we will keep the end of the structure intact
	// TODO Flag an error
	return true;
}

bool icsneo_settingsApplyStructure(const neodevice_t* device, const void* structure, size_t structureSize) {
	return icsneo_settingsWriteStructure(device, structure, structureSize) && icsneo_settingsApply(device);
}

bool icsneo_settingsApplyStructureTemporary(const neodevice_t* device, const void* structure, size_t structureSize) {
	return icsneo_settingsWriteStructure(device, structure, structureSize) && icsneo_settingsApplyTemporary(device);
}

int64_t icsneo_getBaudrate(const neodevice_t* device, uint16_t netid) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return -1;
	}

	return device->device->settings->getBaudrateFor(netid);
}

bool icsneo_setBaudrate(const neodevice_t* device, uint16_t netid, int64_t newBaudrate) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->setBaudrateFor(netid, newBaudrate);
}

int64_t icsneo_getFDBaudrate(const neodevice_t* device, uint16_t netid) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return -1;
	}

	return device->device->settings->getFDBaudrateFor(netid);
}

bool icsneo_setFDBaudrate(const neodevice_t* device, uint16_t netid, int64_t newBaudrate) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->settings->setFDBaudrateFor(netid, newBaudrate);
}

bool icsneo_transmit(const neodevice_t* device, const neomessage_t* message) {
	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	return device->device->transmit(CreateMessageFromNeoMessage(message));
}

bool icsneo_transmitMessages(const neodevice_t* device, const neomessage_t* messages, size_t count) {
	// Transmit implements neodevice_t check so it is not needed here
	// TODO This can be implemented faster
	for(size_t i = 0; i < count; i++) {
		if(!icsneo_transmit(device, messages + i))
			return false;
	}
	return true;
}

bool icsneo_describeDevice(const neodevice_t* device, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	if(!icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	std::string output = device->device->describe();

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		ErrorManager::GetInstance().add(APIError::OutputTruncated);

	return true;
}

neoversion_t icsneo_getVersion(void) {
	return icsneo::GetVersion();
}

bool icsneo_getErrors(neoerror_t* errors, size_t* size) {
	if(size == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	if(errors == nullptr) {
		*size = icsneo::ErrorCount();
		return false;
	}

	auto cppErrors = icsneo::GetErrors(*size);
	for(size_t i = 0; i < cppErrors.size(); i++)
		memcpy(&errors[i], cppErrors[i].getNeoError(), sizeof(neoerror_t));
	*size = cppErrors.size();

	return true;
}

bool icsneo_getDeviceErrors(const neodevice_t* device, neoerror_t* errors, size_t* size) {
	if(device != nullptr && !icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return false;
	}

	if(size == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	// Creating the filter will nullptr is okay! It will find any errors not associated with a device.
	ErrorFilter filter = (device != nullptr ? device->device : nullptr);

	if(errors == nullptr) {
		*size = icsneo::ErrorCount(filter);
		return false;
	}

	auto cppErrors = icsneo::GetErrors(*size, filter);
	for(size_t i = 0; i < cppErrors.size(); i++)
		memcpy(&errors[i], cppErrors[i].getNeoError(), sizeof(neoerror_t));
	*size = cppErrors.size();

	return true;
}

bool icsneo_getLastError(neoerror_t* error) {
	if(error == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	APIError cppErr;
	if(!icsneo::GetLastError(cppErr))
		return false;
	memcpy(error, cppErr.getNeoError(), sizeof(neoerror_t));
	return true;
}

void icsneo_discardAllErrors(void) {
	icsneo::DiscardErrors();
}

void icsneo_discardDeviceErrors(const neodevice_t* device) {
	if(device != nullptr && !icsneo_isValidNeoDevice(device)) {
		ErrorManager::GetInstance().add(APIError::InvalidNeoDevice);
		return;
	}

	if(device == nullptr)
		icsneo::DiscardErrors(nullptr); // Discard errors not associated with a device
	else
		icsneo::DiscardErrors(device->device);
}

void icsneo_setErrorLimit(size_t newLimit) {
	icsneo::SetErrorLimit(newLimit);
}

size_t icsneo_getErrorLimit(void) {
	return icsneo::GetErrorLimit();
}

bool icsneo_getSupportedDevices(devicetype_t* devices, size_t* count) {
	if(count == nullptr) {
		ErrorManager::GetInstance().add(APIError::RequiredParameterNull);
		return false;
	}

	auto supported = DeviceFinder::GetSupportedDevices();
	auto len = supported.size();

	if(devices == nullptr) {
		*count = len;
		return false;
	}

	if(*count < len) {
		len = *count;
		ErrorManager::GetInstance().add(APIError::OutputTruncated);
	}

	for(size_t i = 0; i < len; i++)
		devices[i] = supported[i];
	*count = len;

	return true;
}