#ifndef __SHAREDSEMAPHORE_WINDOWS_H_
#define __SHAREDSEMAPHORE_WINDOWS_H_

#ifdef __cplusplus

#include <string>
#include <chrono>
#include <optional>
#include "icsneo/platform/windows.h"
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

	bool closing = false;
	device_eventhandler_t report = makeEventHandler();
	std::optional<HANDLE> semaphore;
	std::optional<bool> created;
};

}

#endif // __cplusplus

#endif
