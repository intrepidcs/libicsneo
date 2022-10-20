#ifndef __MESSAGECALLBACK_H_
#define __MESSAGECALLBACK_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/filter/messagefilter.h"
#include <memory>
#include <functional>

namespace icsneo {

class MessageCallback {
public:
	typedef std::function< void( std::shared_ptr<Message> ) > fn_messageCallback;

	MessageCallback(fn_messageCallback cb, std::shared_ptr<MessageFilter> f)
		: callback(cb), filter(f ? f : std::make_shared<MessageFilter>()) {
		if(!cb)
			throw std::bad_function_call();
	}

	MessageCallback(fn_messageCallback cb, MessageFilter f = MessageFilter())
		: MessageCallback(cb, std::make_shared<MessageFilter>(f)) {}
	
	// Allow the filter to be placed first if the user wants (maybe in the case of a lambda)
	MessageCallback(std::shared_ptr<MessageFilter> f, fn_messageCallback cb)
		: MessageCallback(cb, f) {}
	MessageCallback(MessageFilter f, fn_messageCallback cb)
		: MessageCallback(cb, std::make_shared<MessageFilter>(f)) {}

	virtual ~MessageCallback() = default;
	
	virtual bool callIfMatch(const std::shared_ptr<Message>& message) const {
		bool ret = filter->match(message);
		if(ret)
			callback(message);
		return ret;
	}
	const MessageFilter& getFilter() const { return *filter; }
	const fn_messageCallback& getCallback() const { return callback; }

protected:
	const fn_messageCallback callback;
	const std::shared_ptr<MessageFilter> filter;
};

}

#endif // __cplusplus

#endif