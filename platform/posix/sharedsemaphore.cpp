#include <iostream> // TODO: Remove later
#include <fcntl.h>
#include "icsneo/platform/posix/sharedsemaphore.h"

using namespace icsneo;

SharedSemaphore::~SharedSemaphore() {
	close();
}

bool SharedSemaphore::open(const std::string& name, bool create, unsigned initialCount) {
	const auto slashPrefixed = "/" + name;
	if(create)
		sem_unlink(slashPrefixed.c_str()); // try clean-up, it's fine if it errors
	const auto sem = create ? sem_open(slashPrefixed.c_str(), O_CREAT | O_EXCL, 0600, initialCount) : sem_open(slashPrefixed.c_str(), 0);
	if(sem == SEM_FAILED) {
		report(APIEvent::Type::SharedSemaphoreFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	mName.emplace(slashPrefixed);
	semaphore.emplace(sem);
	created.emplace(create);
	return true;
}

bool SharedSemaphore::close() {
	closing = true;

	post(); // wake any waiting

	bool failed = false;
	if(semaphore) {
		if(sem_close(semaphore.value()) == -1) {
			report(APIEvent::Type::SharedSemaphoreFailedToClose, APIEvent::Severity::Error);
			failed = true;
		}
		semaphore.reset();
	}
	if(mName && created && *created) {
		if(sem_unlink(mName->c_str()) == -1) {
			report(APIEvent::Type::SharedSemaphoreFailedToUnlink, APIEvent::Severity::Error);
			failed = true;
		}
		mName.reset();
	}
	return !failed;
}

bool SharedSemaphore::wait(const std::chrono::milliseconds& timeout) {
	if(!semaphore) {
		report(APIEvent::Type::SharedSemaphoreNotOpenForWait, APIEvent::Severity::Error);
		return false;
	}
	const auto timedwait = [&]() -> bool {
		#if defined(__MACH__)
		// TODO: Quite inefficient due to Darwin's lack of sem_timedwait()
		const auto tryTill = std::chrono::steady_clock::now() + timeout;
		while (std::chrono::steady_clock::now() <= tryTill) {
			if(sem_trywait(*semaphore) == 0)
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return false;
		#else // UNIX
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += static_cast<unsigned int>(timeout.count() / 1000);
		ts.tv_nsec += static_cast<int>((timeout.count() % 1000) * 1000000);
		// potentially promote another second
		if(ts.tv_nsec >= 1000000000) {
			ts.tv_nsec -= 1000000000;
			++ts.tv_sec;
		}
		return sem_timedwait(*semaphore, &ts) != -1;
		#endif
	};
	if(!timedwait()) {
		if(errno == ETIMEDOUT)
			return false; // unable to lock within timeout
		if(errno != EINTR) // we don't need a warning for this
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
	if(sem_post(*semaphore) == -1) {
		report(APIEvent::Type::SharedSemaphoreFailedToPost, APIEvent::Severity::Error);
		return false;
	}
	return true;
}
