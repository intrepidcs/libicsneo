#include "icsneo/api/eventmanager.h"
#include <memory>

using namespace icsneo;

static std::unique_ptr<EventManager> singleton;

EventManager& EventManager::GetInstance() {
	if(!singleton)
		singleton = std::unique_ptr<EventManager>(new EventManager());
	return *singleton.get();
}

void EventManager::ResetInstance() {
	singleton = std::unique_ptr<EventManager>(new EventManager());
}

// If this thread is not in the map, add it to be ignored
// If it is, set it to be ignored
void EventManager::downgradeErrorsOnCurrentThread() {
	auto i = downgradedThreads.find(std::this_thread::get_id());
	if(i != downgradedThreads.end()) {
		i->second = true;
	} else {
		downgradedThreads.insert({std::this_thread::get_id(), true});
	}
}

// If this thread exists in the map, turn off downgrading
void EventManager::cancelErrorDowngradingOnCurrentThread() {
	auto i = downgradedThreads.find(std::this_thread::get_id());
	if(i != downgradedThreads.end()) {
		i->second = false;
	}
}

void EventManager::get(std::vector<APIEvent>& eventOutput, size_t max, EventFilter filter) {
	std::lock_guard<std::mutex> lk(mutex);
	
	if(max == 0) // A limit of 0 indicates no limit
		max = (size_t)-1;

	size_t count = 0;
	eventOutput.clear();
	auto it = events.begin();
	while(it != events.end()) {
		if(filter.match(*it)) {
			eventOutput.push_back(*it);
			it = events.erase(it);
			if(++count >= max)
				break; // We now have as many written to output as we can
		} else {
			it++;
		}
	}
}

/**
 * Removes the returned error from the map
 * If no error was found, return a NoErrorFound Info event
 */
APIEvent EventManager::getLastError() {
	std::lock_guard<std::mutex> lk(mutex);

	auto it = lastUserErrors.find(std::this_thread::get_id());
	if(it == lastUserErrors.end()) {
		return APIEvent(APIEvent::Type::NoErrorFound, APIEvent::Severity::EventInfo);
	} else {
		APIEvent ret = it->second;
		it = lastUserErrors.erase(it);
		return ret;
	}
}

void EventManager::discard(EventFilter filter) {
	std::lock_guard<std::mutex> lk(mutex);
	events.remove_if([&filter](const APIEvent& event) {
		return filter.match(event);
	});
}

size_t EventManager::count_internal(EventFilter filter) const {
	size_t ret = 0;
	for(auto& event : events)
		if(filter.match(event))
			ret++;
	return ret;
}

/**
 * Ensures events is always at most eventLimit - 1 in size.
 * Returns true if any events were removed in the process of doing so.
 */
bool EventManager::enforceLimit() {
	// Remove all TooManyEvents from the end before checking
	auto filter = EventFilter(APIEvent::Type::TooManyEvents);
	auto it = events.rbegin();
	while(it != events.rend() && filter.match(*it)) {
		it = decltype(it){events.erase( std::next(it).base() )};
	}
	
	// We are not overflowing
	if(events.size() < eventLimit)
		return false;

	size_t amountToRemove = events.size() + 1 - eventLimit;

	discardOldest(amountToRemove);

	return true;
}

void EventManager::discardOldest(size_t count) {
	if(count == 0)
		return;

	auto it = events.begin();
	while(it != events.end()) {
		it = events.erase(it);
		if(--count == 0)
			break;				
	}
}