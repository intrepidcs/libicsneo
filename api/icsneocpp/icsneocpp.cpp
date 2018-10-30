#include "icsneo/icsneocpp.h"
#include "icsneo/device/devicefinder.h"

using namespace icsneo;

std::vector<std::shared_ptr<Device>> icsneo::FindAllDevices() {
	return DeviceFinder::FindAll();
}

size_t icsneo::ErrorCount(ErrorFilter filter) {
	return ErrorManager::GetInstance().count(filter);
}

std::vector<APIError> icsneo::GetErrors(ErrorFilter filter, size_t max) {
	return ErrorManager::GetInstance().get(filter, max);
}

std::vector<APIError> icsneo::GetErrors(size_t max, ErrorFilter filter) {
	return ErrorManager::GetInstance().get(max, filter);
}

void icsneo::GetErrors(std::vector<APIError>& errors, ErrorFilter filter, size_t max) {
	ErrorManager::GetInstance().get(errors, filter, max);
}

void icsneo::GetErrors(std::vector<APIError>& errors, size_t max, ErrorFilter filter) {
	ErrorManager::GetInstance().get(errors, max, filter);
}

bool icsneo::GetLastError(APIError& error, ErrorFilter filter) {
	return ErrorManager::GetInstance().getLastError(error, filter);
}

void icsneo::DiscardErrors(ErrorFilter filter) {
	ErrorManager::GetInstance().discard(filter);
}

void icsneo::SetErrorLimit(size_t newLimit) {
	ErrorManager::GetInstance().setErrorLimit(newLimit);
}

size_t icsneo::GetErrorLimit() {
	return ErrorManager::GetInstance().getErrorLimit();
}