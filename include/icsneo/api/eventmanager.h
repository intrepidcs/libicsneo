#ifndef __ICSNEO_API_EVENTMANAGER_H_
#define __ICSNEO_API_EVENTMANAGER_H_

#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <thread>
#include "icsneo/api/event.h"

namespace icsneo {

typedef std::function<void (APIEvent::Type, APIEvent::Severity)> device_eventhandler_t;

class EventManager {
public:
	static EventManager& GetInstance();

	static void ResetInstance();

	size_t eventCount(EventFilter filter = EventFilter()) const {
		std::lock_guard<std::mutex> lk(mutex);
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
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(event);
	}
	void add(APIEvent::Type type, APIEvent::Severity severity, const Device* forDevice = nullptr) {
		std::lock_guard<std::mutex> lk(mutex);
		add_internal(APIEvent(type, severity, forDevice));
	}

	void discard(EventFilter filter = EventFilter());

	void setEventLimit(size_t newLimit) {
		if(newLimit == eventLimit)
			return;
		
		if(newLimit < 10) {
			add(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
			return;
		}

		std::lock_guard<std::mutex> lk(mutex);
		eventLimit = newLimit;
		if(enforceLimit()) 
			add_internal(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
	}

	size_t getEventLimit() const { return eventLimit; }



private:
	EventManager() : mutex(), events(), lastUserErrors(), eventLimit(10000) {}
	// Used by functions for threadsafety
	mutable std::mutex mutex;

	// Stores all events
	std::list<APIEvent> events;
	std::unordered_map<std::thread::id, APIEvent> lastUserErrors;
	size_t eventLimit = 10000;

	size_t count_internal(EventFilter filter = EventFilter()) const;

	void add_internal(APIEvent event) {
		if(event.getSeverity() == APIEvent::Severity::Error)
			add_internal_error(event);
		else
			add_internal_event(event);
	}

	/**
	 * Places a {id, event} pair into the lastUserErrors
	 * If the key id already exists in the map, replace the event of that pair with the new one
	 */
	void add_internal_error(APIEvent event) {
		auto iter = lastUserErrors.find(std::this_thread::get_id());
		if(iter == lastUserErrors.end())
			lastUserErrors.insert(std::make_pair(std::this_thread::get_id(), event));
		else
			iter->second = event;
	}

	/**
	 * If events is not full, add the event at the end
	 * Otherwise, remove the least significant events, push the event to the back and push a APIEvent::TooManyEvents to the back (in that order)
	 */
	void add_internal_event(APIEvent event) {
		// Ensure the event list is at most exactly full (size of eventLimit - 1, leaving room for a potential APIEvent::TooManyEvents) 
		enforceLimit();

		// We are exactly full, either because the list was truncated or because we were simply full before
		if(events.size() == eventLimit - 1) {
			// If the event is worth adding
			if(event.getSeverity() >= lowestCurrentSeverity()) {
				discardLeastSevere(1);
				events.push_back(event);	
			}

			events.push_back(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
		} else {
			events.push_back(event);
		}
	}

	bool enforceLimit(); // Returns whether the limit enforcement resulted in an overflow

	APIEvent::Severity lowestCurrentSeverity();
	void discardLeastSevere(size_t count = 1);
};

}

#endif