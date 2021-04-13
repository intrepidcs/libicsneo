#ifndef __cplusplus
#error "icsneoc.cpp must be compiled with a C++ compiler!"
#endif

#define ICSNEOC_MAKEDLL

#include "icsneo/icsneoc.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/platform/dynamiclib.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/device/devicefinder.h"
#include <string>
#include <functional>
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
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
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
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);
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
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	auto result = Device::SerialNumToString(num);

	if(str == nullptr) {
		*count = result.length() + 1;
		return false;
	}

	if(*count < result.length()) {
		*count = result.length() + 1; // This is how big of a buffer we need
		EventManager::GetInstance().add(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error);
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
	// return false on nullptr
	if(!device) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}
	// If this neodevice_t was returned by a previous search, it will no longer be valid (as the underlying icsneo::Device is freed)
	for(auto& dev : connectedDevices) {
		if(dev.get() == device->device)
			return true;
	}
	for(auto& dev : connectableFoundDevices) {
		if(dev.get() == device->device)
			return true;
	}

	EventManager::GetInstance().add(APIEvent::Type::InvalidNeoDevice, APIEvent::Severity::Error);
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

bool icsneo_isOpen(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->isOpen();
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

	return device->device->enableMessagePolling();
}

bool icsneo_disableMessagePolling(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->disableMessagePolling();
}

bool icsneo_isMessagePollingEnabled(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return false;
	
	return device->device->isMessagePollingEnabled();
}

bool icsneo_getMessages(const neodevice_t* device, neomessage_t* messages, size_t* items, uint64_t timeout) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	if(items == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
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

int icsneo_getPollingMessageLimit(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return -1;

	return (int)device->device->getPollingMessageLimit();
}

bool icsneo_setPollingMessageLimit(const neodevice_t* device, size_t newLimit) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	device->device->setPollingMessageLimit(newLimit);
	return true;
}

int icsneo_addMessageCallback(const neodevice_t* device, void (*callback)(neomessage_t), void*) {
	if(!icsneo_isValidNeoDevice(device))
		return -1;

	return device->device->addMessageCallback(
		MessageCallback(
			[=](std::shared_ptr<icsneo::Message> msg) {
				return callback(CreateNeoMessage(msg));
			}
		)
	);
}

bool icsneo_removeMessageCallback(const neodevice_t* device, int id) {
	if(!icsneo_isValidNeoDevice(device))
		return false;
	return device->device->removeMessageCallback(id);
}

bool icsneo_getProductName(const neodevice_t* device, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	if(!icsneo_isValidNeoDevice(device))
		return false;

	std::string output = device->device->getProductName();

	if(str == nullptr) {
		*maxLength = output.length();
		return false;
	}

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);

	return true;
}

bool icsneo_getProductNameForType(devicetype_t type, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	std::string output = DeviceType(type).getGenericProductName();

	if(str == nullptr) {
		*maxLength = output.length();
		return false;
	}

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);

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

int icsneo_settingsReadStructure(const neodevice_t* device, void* structure, size_t structureSize) {
	if(!icsneo_isValidNeoDevice(device))
		return -1;

	size_t readSize = device->device->settings->getSize();
	if(structure == nullptr) // Structure size request
		return (int)readSize;
	if(readSize > structureSize) {
		// Client application has a smaller structure than we do
		// It is probably built against an older version of the API
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);
		readSize = structureSize;
	}

	const void* deviceStructure = device->device->settings->getRawStructurePointer();
	if(deviceStructure == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return -1;
	}

	memcpy(structure, deviceStructure, readSize);
	
	if(readSize < structureSize) // Client application is attempting to read more than we have
		memset((uint8_t*)structure + readSize, 0, structureSize - readSize);

	return (int)readSize;
}

// Not exported
static bool icsneo_settingsWriteStructure(const neodevice_t* device, const void* structure, size_t structureSize) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	if(structure == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	size_t writeSize = device->device->settings->getSize();
	if(writeSize < structureSize) {
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);
		structureSize = writeSize;
	}

	void* deviceStructure = device->device->settings->getMutableRawStructurePointer();
	if(deviceStructure == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
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

int64_t icsneo_getBaudrate(const neodevice_t* device, neonetid_t netid) {
	if(!icsneo_isValidNeoDevice(device))
		return -1;

	return device->device->settings->getBaudrateFor(netid);
}

bool icsneo_setBaudrate(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->setBaudrateFor(netid, newBaudrate);
}

int64_t icsneo_getFDBaudrate(const neodevice_t* device, neonetid_t netid) {
	if(!icsneo_isValidNeoDevice(device))
		return -1;

	return device->device->settings->getFDBaudrateFor(netid);
}

bool icsneo_setFDBaudrate(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->setFDBaudrateFor(netid, newBaudrate);
}

bool icsneo_transmit(const neodevice_t* device, const neomessage_t* message) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

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

void icsneo_setWriteBlocks(const neodevice_t* device, bool blocks) {
	if(!icsneo_isValidNeoDevice(device))
		return;
	
	device->device->setWriteBlocks(blocks);
}

bool icsneo_describeDevice(const neodevice_t* device, char* str, size_t* maxLength) {
	// TAG String copy function
	if(maxLength == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	if(!icsneo_isValidNeoDevice(device))
		return false;

	std::string output = device->device->describe();

	*maxLength = output.copy(str, *maxLength);
	str[*maxLength] = '\0';

	if(output.length() > *maxLength)
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);

	return true;
}

neoversion_t icsneo_getVersion(void) {
	return icsneo::GetVersion();
}

int icsneo_addEventCallback(void (*callback)(neoevent_t), void*) {
	return EventManager::GetInstance().addEventCallback(
		EventCallback(
			[=](std::shared_ptr<icsneo::APIEvent> evt) {
				return callback(*(evt->getNeoEvent()));
			}
		)
	);
}

bool icsneo_removeEventCallback(int id) {
	return EventManager::GetInstance().removeEventCallback(id);
}

bool icsneo_getEvents(neoevent_t* events, size_t* size) {
	if(size == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	if(events == nullptr) {
		*size = icsneo::EventCount();
		return false;
	}

	auto cppErrors = icsneo::GetEvents(*size);
	for(size_t i = 0; i < cppErrors.size(); i++)
		memcpy(&events[i], cppErrors[i].getNeoEvent(), sizeof(neoevent_t));
	*size = cppErrors.size();

	return true;
}

bool icsneo_getDeviceEvents(const neodevice_t* device, neoevent_t* events, size_t* size) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	if(size == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	// Creating the filter will nullptr is okay! It will find any events not associated with a device.
	EventFilter filter = (device != nullptr ? device->device : nullptr);

	if(events == nullptr) {
		*size = icsneo::EventCount(filter);
		return false;
	}

	auto cppErrors = icsneo::GetEvents(*size, filter);
	for(size_t i = 0; i < cppErrors.size(); i++)
		memcpy(&events[i], cppErrors[i].getNeoEvent(), sizeof(neoevent_t));
	*size = cppErrors.size();

	return true;
}

bool icsneo_getLastError(neoevent_t* error) {
	if(error == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	APIEvent cppErr = icsneo::GetLastError();
	if(cppErr.getType() == icsneo::APIEvent::Type::NoErrorFound)
		return false;
	memcpy(error, cppErr.getNeoEvent(), sizeof(neoevent_t));
	return true;
}

void icsneo_discardAllEvents(void) {
	icsneo::DiscardEvents();
}

void icsneo_discardDeviceEvents(const neodevice_t* device) {
	if(!icsneo_isValidNeoDevice(device))
		return;

	if(device == nullptr)
		icsneo::DiscardEvents(nullptr); // Discard events not associated with a device
	else
		icsneo::DiscardEvents(device->device);
}

void icsneo_setEventLimit(size_t newLimit) {
	icsneo::SetEventLimit(newLimit);
}

size_t icsneo_getEventLimit(void) {
	return icsneo::GetEventLimit();
}

bool icsneo_getSupportedDevices(devicetype_t* devices, size_t* count) {
	if(count == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
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
		EventManager::GetInstance().add(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning);
	}

	for(size_t i = 0; i < len; i++)
		devices[i] = supported[i];
	*count = len;

	return true;
}

bool icsneo_getTimestampResolution(const neodevice_t* device, uint16_t* resolution) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	if(resolution == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	*resolution = device->device->getTimestampResolution();
	return true;
}

bool icsneo_getDigitalIO(const neodevice_t* device, neoio_t type, uint32_t number, bool* value) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	if(value == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return false;
	}

	const optional<bool> val = device->device->getDigitalIO(static_cast<icsneo::IO>(type), number);
	if(!val.has_value())
		return false;

	*value = *val;
	return true;
}

bool icsneo_setDigitalIO(const neodevice_t* device, neoio_t type, uint32_t number, bool value) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->setDigitalIO(static_cast<icsneo::IO>(type), number, value);
}

bool icsneo_isTerminationSupportedFor(const neodevice_t* device, neonetid_t netid) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->isTerminationSupportedFor(Network(netid));
}

bool icsneo_canTerminationBeEnabledFor(const neodevice_t* device, neonetid_t netid) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->canTerminationBeEnabledFor(Network(netid));
}

bool icsneo_isTerminationEnabledFor(const neodevice_t* device, neonetid_t netid) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->isTerminationEnabledFor(Network(netid)).value_or(false);
}

bool icsneo_setTerminationFor(const neodevice_t* device, neonetid_t netid, bool enabled) {
	if(!icsneo_isValidNeoDevice(device))
		return false;

	return device->device->settings->setTerminationFor(Network(netid), enabled);
}
