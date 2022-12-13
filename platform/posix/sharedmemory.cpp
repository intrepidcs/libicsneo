#include <iostream> // TODO: Remove later
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "icsneo/platform/posix/sharedmemory.h"

using namespace icsneo;

SharedMemory::~SharedMemory() {
	close();
}

bool SharedMemory::open(const std::string& name, uint32_t size, bool create) {
	if(create)
		shm_unlink(name.c_str());
	const auto fd = create ? shm_open(name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0600) : shm_open(name.c_str(), O_RDWR, 0);
	if(fd == -1) {
		report(APIEvent::Type::SharedMemoryFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	mName.emplace(name);
	if(create && ftruncate(fd, size) == -1) {
		report(APIEvent::Type::SharedMemoryFileTruncateError, APIEvent::Severity::Error);
		close();
		return false;
	}
	const auto shm = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(shm == MAP_FAILED) {
		report(APIEvent::Type::SharedMemoryMappingError, APIEvent::Severity::Error);
		close();
		return false;
	}
	mData.emplace(std::make_pair((uint8_t*)shm, size));
	mCreated.emplace(create);
	return true;
}

bool SharedMemory::close() {
	bool failed = false;
	if(mData) {
		if(munmap((void*)mData->first, mData->second) == -1) {
			report(APIEvent::Type::SharedMemoryUnmapError, APIEvent::Severity::EventWarning);
			failed = true;
		}
		mData.reset();
	}
	if(mName && mCreated && *mCreated) {
		if(shm_unlink(mName->c_str()) == -1) {
			report(APIEvent::Type::SharedMemoryFailedToUnlink, APIEvent::Severity::EventWarning);
			failed = true;
		}
		mName.reset();
	}
	if(failed)
		report(APIEvent::Type::SharedMemoryFailedToClose, APIEvent::Severity::Error);
	return !failed;
}

uint8_t* SharedMemory::data() {
	if(!mData) {
		report(APIEvent::Type::SharedMemoryDataIsNull, APIEvent::Severity::Error);
		return nullptr;
	}
	return mData->first;
}
