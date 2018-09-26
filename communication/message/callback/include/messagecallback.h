#ifndef __MESSAGECALLBACK_H_
#define __MESSAGECALLBACK_H_

#include "communication/message/include/message.h"
#include "communication/message/filter/include/messagefilter.h"
#include <memory>
#include <functional>

namespace icsneo {

class MessageCallback {
public:
	typedef std::function< void( std::shared_ptr<Message> ) > fn_messageCallback;

	MessageCallback(fn_messageCallback cb, std::shared_ptr<MessageFilter> f) : callback(cb), filter(f) {}
	MessageCallback(fn_messageCallback cb, MessageFilter f = MessageFilter()) : callback(cb), filter(std::make_shared<MessageFilter>(f)) {}
	
	// Allow the filter to be placed first if the user wants (maybe in the case of a lambda)
	MessageCallback(MessageFilter f, fn_messageCallback cb) { MessageCallback(cb, f); }
	
	virtual bool callIfMatch(const std::shared_ptr<Message>& message) const {
		bool ret = filter->match(message);
		if(ret)
			callback(message);
		return ret;
	}
	const MessageFilter& getFilter() const { return *filter; }
	const fn_messageCallback& getCallback() const { return callback; }

protected:
	fn_messageCallback callback;
	std::shared_ptr<MessageFilter> filter;
};

}

#endif