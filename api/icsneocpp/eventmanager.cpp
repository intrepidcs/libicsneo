#include "icsneo/api/eventmanager.h"
#include "icsneo/api/event.h"
#include <memory>
#include <thread>

#ifdef ICSNEO_ENABLE_DEVICE_SHARING
#include "icsneo/communication/socket.h"
#endif

using namespace icsneo;

EventManager& EventManager::GetInstance() {
	static EventManager inst;
	return inst;
}

void EventManager::downgradeErrorsOnCurrentThread() {
	if(destructing)
		return;
	std::lock_guard<std::mutex> lk(downgradedThreadsMutex);
	auto i = downgradedThreads.find(std::this_thread::get_id());
	if(i != downgradedThreads.end()) {
		i->second = true;
	} else {
		downgradedThreads.insert({std::this_thread::get_id(), true});
	}
}

void EventManager::cancelErrorDowngradingOnCurrentThread() {
	if(destructing)
		return;
	std::lock_guard<std::mutex> lk(downgradedThreadsMutex);
	auto i = downgradedThreads.find(std::this_thread::get_id());
	if(i != downgradedThreads.end()) {
		i->second = false;
	}
}

void EventManager::add(APIEvent event) {
	if(destructing)
		return;
	if(event.getSeverity() == APIEvent::Severity::Error) {
		// if the error was added on a thread that downgrades errors (non-user thread)
		std::lock_guard<std::mutex> lk(downgradedThreadsMutex);
		auto i = downgradedThreads.find(std::this_thread::get_id());
		if(i != downgradedThreads.end() && i->second) {
			event.downgradeFromError();
			{
				std::lock_guard<std::mutex> eventsLock{eventsMutex};
				addEventInternal(event);
			} // free the lock so that callbacks may modify events
			runCallbacks(event);
		} else {
			std::lock_guard<std::mutex> errorsLock(errorsMutex);
			lastUserErrors[std::this_thread::get_id()] = event;
		}
	} else {
		{
			std::lock_guard<std::mutex> eventsLock(eventsMutex);
			addEventInternal(event);
		} // free the lock so that callbacks may modify events
		runCallbacks(event);
	}
}

void EventManager::addEventInternal(APIEvent event) {
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

void EventManager::runCallbacks(APIEvent event) {
	std::lock_guard<std::mutex> lk(callbacksMutex);
	for(auto& i : callbacks)
		i.second.callIfMatch(std::make_shared<APIEvent>(event));
}

void EventManager::setEventLimit(size_t newLimit) {
	std::lock_guard<std::mutex> eventLimitLock(eventLimitMutex);
	
	if(newLimit == eventLimit)
		return;
	
	if(newLimit < 10) {
		add(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return;
	}

	eventLimit = newLimit;
	
	std::lock_guard<std::mutex> eventsLock(eventsMutex);
	if(enforceLimit())
		addEventInternal(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
}

void EventManager::ResetInstance() {
	std::lock_guard<std::mutex> eventsLock(eventsMutex);
	std::lock_guard<std::mutex> errorsLock(errorsMutex);
	std::lock_guard<std::mutex> downgradedThreadsLock(downgradedThreadsMutex);
	std::lock_guard<std::mutex> callbacksLock(callbacksMutex);

	std::lock_guard<std::mutex> callbackIDLock(callbackIDMutex);
	std::lock_guard<std::mutex> eventLimitLock(eventLimitMutex);

	events.clear();
	lastUserErrors.clear();
	downgradedThreads.clear();
	callbacks.clear();

	callbackID = 0;
	eventLimit = 10000;
}

int EventManager::addEventCallback(const EventCallback &cb) {
	std::lock_guard<std::mutex> callbacksLock(callbacksMutex);
	std::lock_guard<std::mutex> callbackIDLock(callbackIDMutex);
	callbacks.insert({callbackID, cb});
	return callbackID++;
}

bool EventManager::removeEventCallback(int id) {
	std::lock_guard<std::mutex> lk(callbacksMutex);

	auto iter = callbacks.find(id);

	if(iter != callbacks.end()) {
		callbacks.erase(iter);
		return true;
	} else
		return false;
}

bool EventManager::isDowngradingErrorsOnCurrentThread() const {
	auto i = downgradedThreads.find(std::this_thread::get_id());
	if(i != downgradedThreads.end()) {
		return i->second;
	}
	return false;
}

#ifdef ICSNEO_ENABLE_DEVICE_SHARING
std::optional<std::vector<neosocketevent_t>> EventManager::getServerEvents(const size_t& max)
{
	auto socket = lockSocket();
	if(!(socket.writeTyped<RPC>(RPC::GET_EVENTS) && socket.writeTyped<size_t>(max)))
		return std::nullopt;

	size_t count;
	if(!socket.readTyped(count) || count < 0 || count > eventLimit)
		return std::nullopt;

	std::optional<std::vector<neosocketevent_t>> ret;
	if(count == size_t(0))
		return ret;

	auto& eventStructs = ret.emplace(count);
	if(!socket.read(eventStructs.data(), (eventStructs.size() * sizeof(neosocketevent_t))))
		return std::nullopt;

	return ret;
}
#endif // ICSNEO_ENABLE_DEVICE_SHARING

void EventManager::get(std::vector<APIEvent>& eventOutput, size_t max, EventFilter filter) {
	eventOutput.clear();
	#ifdef ICSNEO_ENABLE_DEVICE_SHARING
	{
		auto serverEvents = getServerEvents(max);
		if(serverEvents) {
			std::scoped_lock eventsLock{eventsMutex};
			for(neosocketevent_t& event : *serverEvents) {
				eventOutput.emplace_back(event);
			}
			if(max != 0u)
				max -= eventOutput.size();
		}
	}
	#endif
	std::scoped_lock eventsLock{eventsMutex};
	{
		if(max == 0) // A limit of 0 indicates no limit
			max = (size_t)-1;

		size_t count = 0;
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
}

/**
 * Removes the returned error from the map
 * If no error was found, return a NoErrorFound Info event
 */
APIEvent EventManager::getLastError() {
	std::lock_guard<std::mutex> lk(errorsMutex);

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
	std::lock_guard<std::mutex> lk(eventsMutex);
	events.remove_if([&filter](const APIEvent& event) {
		return filter.match(event);
	});
}

size_t EventManager::countInternal(EventFilter filter) const {
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