#ifndef __ICSNEO_API_EVENTMANAGER_H_
#define __ICSNEO_API_EVENTMANAGER_H_

#ifdef __cplusplus

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include <map>
#include <thread>
#include <algorithm>
#include "icsneo/api/event.h"
#include "icsneo/api/eventcallback.h"

namespace icsneo {

typedef std::function<void (APIEvent::Type, APIEvent::Severity)> device_eventhandler_t;

class EventManager {
public:
	~EventManager() {
		destructing = true;
	}

	static EventManager& GetInstance();

	void ResetInstance();

	// If this thread is not in the map, add it to be ignored
	// If it is, set it to be ignored
	void downgradeErrorsOnCurrentThread();
	
	// If this thread exists in the map, turn off downgrading
	void cancelErrorDowngradingOnCurrentThread();

	bool isDowngradingErrorsOnCurrentThread() const;

	int addEventCallback(const EventCallback &cb);
	bool removeEventCallback(int id);

	size_t eventCount(EventFilter filter = EventFilter()) const {
		std::lock_guard<std::mutex> lk(eventsMutex);
		return countInternal(filter);
	};

	std::vector<APIEvent> get(EventFilter filter, size_t max = 0) { return get(max, filter); }
	std::vector<APIEvent> get(size_t max = 0, EventFilter filter = EventFilter()) {
		std::vector<APIEvent> ret;
		get(ret, filter, max);
		return ret;
	}
	void get(std::vector<APIEvent>& outEvents, EventFilter filter, size_t max = 0) { get(outEvents, max, filter); }
	void get(std::vector<APIEvent>& outEvents, size_t max = 0, EventFilter filter = EventFilter());
	
	APIEvent getLastError();

	void add(APIEvent event);
	void add(APIEvent::Type type, APIEvent::Severity severity, const Device* forDevice = nullptr) {
		add(APIEvent(type, severity, forDevice));
	}

	void discard(EventFilter filter = EventFilter());

	size_t getEventLimit() const {
		std::lock_guard<std::mutex> lk(eventLimitMutex);
		return eventLimit;
	}
	void setEventLimit(size_t newLimit);

private:
	EventManager() : eventLimit(10000) {}
	EventManager(const EventManager& other); // = delete (not supported everywhere)
	EventManager& operator=(const EventManager& other); // = delete (not supported everywhere)

	// Used by functions for threadsafety
	mutable std::mutex eventsMutex;
	mutable std::mutex errorsMutex;
	mutable std::mutex downgradedThreadsMutex;
	mutable std::mutex callbacksMutex;

	mutable std::mutex callbackIDMutex;
	mutable std::mutex eventLimitMutex;

	std::map<std::thread::id, bool> downgradedThreads;

	std::map<int, EventCallback> callbacks;

	int callbackID = 1;
	
	bool destructing = false;

	// Stores all events
	std::list<APIEvent> events;
	std::map<std::thread::id, APIEvent> lastUserErrors;
	size_t eventLimit = 10000;

	size_t countInternal(EventFilter filter = EventFilter()) const;

	void runCallbacks(APIEvent event);

	/**
	 * If events is not full, add the event at the end
	 * Otherwise, remove the oldest event, push the event to the back and push a APIEvent::TooManyEvents to the back (in that order)
	 */
	void addEventInternal(APIEvent event);

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	void discardOldest(size_t count = 1);
};

}

#endif // __cplusplus

#endif