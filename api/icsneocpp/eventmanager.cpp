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

	discardLeastSevere(amountToRemove);

	return true;
}

APIEvent::Severity EventManager::lowestCurrentSeverity() const {
	if(events.empty())
		return APIEvent::Severity(0);

	APIEvent::Severity lowest = APIEvent::Severity::Error;
	auto it = events.begin();
	while(it != events.end()) {
		if((*it).getSeverity() < lowest)
			lowest = (*it).getSeverity();
		it++;
		
		if(lowest == APIEvent::Severity::EventInfo)
			return lowest;
	}
	return lowest;
}

void EventManager::discardLeastSevere(size_t count) {
	if(count == 0)
		return;

	// Erase needed Info level events, starting from the beginning
	EventFilter infoFilter(APIEvent::Severity::EventInfo);
	auto it = events.begin();
	while(it != events.end()) {
		if(infoFilter.match(*it)) {
			it = events.erase(it);
			if(--count == 0)
				break;
		} else {
			it++;
		}
	}

	// Erase needed Warning level events, starting from the beginning
	if(count != 0) {
		EventFilter warningFilter(APIEvent::Severity::EventWarning);
		it = events.begin();
		while(it != events.end()) {
			if(warningFilter.match(*it)) {
				it = events.erase(it);
				if(--count == 0)
					break;
			} else {
				it++;
			}
		}
	}

	// No need to check for Error level events, as they are not stored in the list of events.
}