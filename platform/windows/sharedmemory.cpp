#include "icsneo/api/event.h"
#include "icsneo/platform/windows/sharedmemory.h"

using namespace icsneo;

SharedMemory::~SharedMemory() {
	close();
}

bool SharedMemory::open(const std::string& name, uint32_t size, bool create) {
	HANDLE shm;
	if(create)
		shm = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name.c_str());
	else
		shm = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());

	if(shm == NULL) {
		report(APIEvent::Type::SharedMemoryFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	mHandle.emplace(shm);
	const auto dataStart = MapViewOfFile(shm, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if(dataStart == NULL) {
		report(APIEvent::Type::SharedMemoryMappingError, APIEvent::Severity::Error);
		close();
		return false;
	}
	mData.emplace(std::make_pair((uint8_t*)dataStart, size));
	mCreated.emplace(create);
	return true;
}

bool SharedMemory::close() {
	bool failed = false; // TODO: need to close properly
	if(failed) {
		report(APIEvent::Type::SharedMemoryFailedToClose, APIEvent::Severity::Error);
	}
	return !failed;
}

uint8_t* SharedMemory::data() {
	if(!mData) {
		report(APIEvent::Type::SharedMemoryDataIsNull, APIEvent::Severity::Error);
		return nullptr;
	}
	return mData->first;
}
