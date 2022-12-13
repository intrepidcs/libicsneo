#ifndef __SHAREDSEMAPHORE_POSIX_H_
#define __SHAREDSEMAPHORE_POSIX_H_

#ifdef __cplusplus

#include <string>
#include <optional>
#include <atomic>
#include <semaphore.h>
#include "icsneo/api/eventmanager.h"

namespace icsneo {

class SharedSemaphore {
public:
	~SharedSemaphore();
	bool open(const std::string& name, bool create = false, unsigned initialCount = 0);
	bool close();
	bool wait(const std::chrono::milliseconds& timeout);
	bool post();

private:
	virtual device_eventhandler_t makeEventHandler() {
		return [](APIEvent::Type type, APIEvent::Severity severity)
		{ EventManager::GetInstance().add(type, severity); };
	}

	std::atomic<bool> closing = false;
	std::atomic<bool> waiting = false;
	device_eventhandler_t report = makeEventHandler();
	std::optional<const std::string> mName;
	std::optional<sem_t*> semaphore;
	std::optional<const bool> created;
};

}

#endif // __cplusplus

#endif
