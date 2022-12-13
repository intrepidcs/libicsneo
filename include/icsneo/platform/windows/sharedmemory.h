#ifndef __SHAREDMEMORY_WINDOWS_H_
#define __SHAREDMEMORY_WINDOWS_H_

#ifdef __cplusplus

#include <string>
#include <optional>
#include "icsneo/platform/windows.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

class SharedMemory {
public:
	SharedMemory() : report(makeEventHandler()) {};
	~SharedMemory();
	bool open(const std::string& name, uint32_t size, bool create = false);
	bool close();
	uint8_t* data();

private:
	virtual device_eventhandler_t makeEventHandler() {
		return [](APIEvent::Type type, APIEvent::Severity severity)
		{ EventManager::GetInstance().add(type, severity); };
	}

	device_eventhandler_t report;
	std::optional<HANDLE> mHandle;
	std::optional<std::pair<uint8_t*, uint32_t>> mData;
	std::optional<bool> mCreated;
};

}

#endif // __cplusplus

#endif
