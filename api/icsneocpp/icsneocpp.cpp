#include "icsneo/icsneocpp.h"
#include "icsneo/device/devicefinder.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> icsneo::FindAllDevices() {
	return DeviceFinder::FindAll();
}

std::vector<DeviceType> icsneo::GetSupportedDevices() {
	return DeviceFinder::GetSupportedDevices();
}

size_t icsneo::EventCount(EventFilter filter) {
	return EventManager::GetInstance().eventCount(filter);
}

std::vector<APIEvent> icsneo::GetEvents(EventFilter filter, size_t max) {
	return EventManager::GetInstance().get(filter, max);
}

std::vector<APIEvent> icsneo::GetEvents(size_t max, EventFilter filter) {
	return EventManager::GetInstance().get(max, filter);
}

void icsneo::GetEvents(std::vector<APIEvent>& events, EventFilter filter, size_t max) {
	EventManager::GetInstance().get(events, filter, max);
}

void icsneo::GetEvents(std::vector<APIEvent>& events, size_t max, EventFilter filter) {
	EventManager::GetInstance().get(events, max, filter);
}

APIEvent icsneo::GetLastError() {
	return EventManager::GetInstance().getLastError();
}

void icsneo::DiscardEvents(EventFilter filter) {
	EventManager::GetInstance().discard(filter);
}

void icsneo::SetEventLimit(size_t newLimit) {
	EventManager::GetInstance().setEventLimit(newLimit);
}

size_t icsneo::GetEventLimit() {
	return EventManager::GetInstance().getEventLimit();
}