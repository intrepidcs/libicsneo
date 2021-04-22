#ifndef __EVENTCALLBACK_H_
#define __EVENTCALLBACK_H_

#ifdef __cplusplus

#include <functional>
#include <memory>
#include "event.h"

namespace icsneo {

class EventCallback {
public:
	typedef std::function< void( std::shared_ptr<APIEvent> ) > fn_eventCallback;

	EventCallback(fn_eventCallback cb, std::shared_ptr<EventFilter> f) : callback(cb), filter(f) {}
	EventCallback(fn_eventCallback cb, EventFilter f = EventFilter()) : callback(cb), filter(std::make_shared<EventFilter>(f)) {}

	// Allow the filter to be placed first if the user wants (maybe in the case of a lambda)
	EventCallback(std::shared_ptr<EventFilter> f, fn_eventCallback cb) : callback(cb), filter(f) {}
	EventCallback(EventFilter f, fn_eventCallback cb) : callback(cb), filter(std::make_shared<EventFilter>(f)) {}

	virtual bool callIfMatch(const std::shared_ptr<APIEvent>& event) const {
		bool ret = filter->match(*event);
		if(ret)
			callback(event);
		return ret;
	}

	const EventFilter& getFilter() const { return *filter; }
	const fn_eventCallback& getCallback() const { return callback; }

private:
	fn_eventCallback callback;
	std::shared_ptr<EventFilter> filter;

};

}

#endif // __cplusplus

#endif