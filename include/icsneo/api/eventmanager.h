#ifndef __ICSNEO_API_EVENTMANAGER_H_
#define __ICSNEO_API_EVENTMANAGER_H_

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include<map>
#include <thread>
#include <algorithm>
#include "icsneo/api/event.h"
#include "icsneo/api/eventcallback.h"

namespace icsneo {

typedef std::function<void (APIEvent::Type, APIEvent::Severity)> device_eventhandler_t;

class EventManager {
public:
	static EventManager& GetInstance();

	static void ResetInstance();

	void downgradeErrorsOnCurrentThread();
	
	void cancelErrorDowngradingOnCurrentThread();

	int addEventCallback(const EventCallback &cb);

	bool removeEventCallback(int id);

	size_t eventCount(EventFilter filter = EventFilter()) const {
		std::lock_guard<std::mutex> lk(eventsMutex);
		return count_internal(filter);
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

	void add(APIEvent event) {
		if(event.getSeverity() == APIEvent::Severity::Error) {
			// if the error was added on a thread that downgrades errors (non-user thread)
			std::lock_guard<std::mutex> lk(downgradedThreadsMutex);
			auto i = downgradedThreads.find(std::this_thread::get_id());
			if(i != downgradedThreads.end() && i->second) {
				event.downgradeFromError();
				std::unique_lock<std::mutex> eventsLock(eventsMutex);
				add_internal_event(event);
				// free the lock so that callbacks may modify events
				eventsLock.unlock();
				runCallbacks(event);
			} else {
				std::lock_guard<std::mutex> errorsLock(errorsMutex);
				add_internal_error(event);
			}
		} else {
			std::unique_lock<std::mutex> eventsLock(eventsMutex);
			add_internal_event(event);
			// free the lock so that callbacks may modify events
			eventsLock.unlock();
			runCallbacks(event);
		}
	}
	void add(APIEvent::Type type, APIEvent::Severity severity, const Device* forDevice = nullptr) {
		add(APIEvent(type, severity, forDevice));
	}

	void discard(EventFilter filter = EventFilter());

	void setEventLimit(size_t newLimit) {
		std::lock_guard<std::mutex> lk(eventsMutex);
		
		if(newLimit == eventLimit)
			return;
		
		if(newLimit < 10) {
			add(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return;
		}
	
		eventLimit = newLimit;
		if(enforceLimit()) 
			add_internal_event(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
	}

	size_t getEventLimit() const { 
		std::lock_guard<std::mutex> lk(eventsMutex);
		return eventLimit;
	}

private:
	EventManager() : eventsMutex(), errorsMutex(), downgradedThreadsMutex(), callbacksMutex(), downgradedThreads(), callbacks(), events(), lastUserErrors(), eventLimit(10000) {}
	EventManager(const EventManager &other);
	EventManager& operator=(const EventManager &other);

	// Used by functions for threadsafety
	mutable std::mutex eventsMutex;
	mutable std::mutex errorsMutex;
	mutable std::mutex downgradedThreadsMutex;
	mutable std::mutex callbacksMutex;

	std::map<std::thread::id, bool> downgradedThreads;

	std::map<int, EventCallback> callbacks;

	int callbackID = 0;
	
	// Stores all events
	std::list<APIEvent> events;
	std::map<std::thread::id, APIEvent> lastUserErrors;
	size_t eventLimit = 10000;

	size_t count_internal(EventFilter filter = EventFilter()) const;

	void runCallbacks(APIEvent event) {
		std::lock_guard<std::mutex> lk(callbacksMutex);
		for(auto &i : callbacks) {
			i.second.callIfMatch(std::make_shared<APIEvent>(event));
		}
	}

	/**
	 * Places a {id, event} pair into the lastUserErrors
	 * If the key id already exists in the map, replace the event of that pair with the new one
	 */
	void add_internal_error(APIEvent event) {
		std::thread::id id = std::this_thread::get_id();
		auto it = lastUserErrors.find(id);
		if(it == lastUserErrors.end())
			lastUserErrors.insert({id, event});
		else
			it->second = event;
	}

	/**
	 * If events is not full, add the event at the end
	 * Otherwise, remove the oldest event, push the event to the back and push a APIEvent::TooManyEvents to the back (in that order)
	 */
	void add_internal_event(APIEvent event) {
		// Ensure the event list is at most exactly full (size of eventLimit - 1, leaving room for a potential APIEvent::TooManyEvents) 
		// Removes any events of type TooManyEvents from the end before checking to avoid duplicates.
		enforceLimit();

		// We are exactly full, either because the list was truncated or because we were simply full before
		if(events.size() == eventLimit - 1) {
			// If the event is worth adding
			if(event.getType() != APIEvent::Type::TooManyEvents) {
				discardOldest(1);
				events.push_back(event);	
			}

			events.push_back(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
		} else {
			if (event.getType() != APIEvent::Type::TooManyEvents)
				events.push_back(event);
		}
	}

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	void discardOldest(size_t count = 1);
};

}

#endif