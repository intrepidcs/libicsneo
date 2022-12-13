#include "icsneo/platform/windows/sharedsemaphore.h"

using namespace icsneo;

SharedSemaphore::~SharedSemaphore() {
	close();
}

bool SharedSemaphore::open(const std::string& name, bool create, unsigned initialCount) {
	HANDLE sem;
	if(create)
		sem = CreateSemaphore(NULL, initialCount, LONG_MAX, name.c_str());
	else
		sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name.c_str());

	if(sem == NULL) {
		report(APIEvent::Type::SharedSemaphoreFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	semaphore.emplace(sem);
	created.emplace(create);
	return true;
}

bool SharedSemaphore::close() {
	if(!semaphore)
		return false;

	closing = true;

	post(); // wake any waiting

	if(CloseHandle(*semaphore) == 0) {
		report(APIEvent::Type::SharedSemaphoreFailedToClose, APIEvent::Severity::Error);
		return false;
	}
	semaphore.reset();
	return true;
}

bool SharedSemaphore::wait(const std::chrono::milliseconds& timeout) {
	if(!semaphore) {
		report(APIEvent::Type::SharedSemaphoreNotOpenForWait, APIEvent::Severity::Error);
		return false;
	}
	if(WaitForSingleObject(*semaphore, static_cast<DWORD>(timeout.count())) != 0) {
		report(APIEvent::Type::SharedSemaphoreFailedToWait, APIEvent::Severity::Error);
		return false;
	}
	if(closing)
		return false; // we were woken by close()
	return true;
}

bool SharedSemaphore::post() {
	if(!semaphore) {
		report(APIEvent::Type::SharedSemaphoreNotOpenForPost, APIEvent::Severity::Error);
		return false;
	}
	if(ReleaseSemaphore(*semaphore, 1, NULL) == 0) {
		report(APIEvent::Type::SharedSemaphoreFailedToPost, APIEvent::Severity::Error);
		return false;
	}
	return true;
}
