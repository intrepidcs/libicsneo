#ifndef __CANMESSAGECALLBACK_H_
#define __CANMESSAGECALLBACK_H_

#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/filter/canmessagefilter.h"
#include <memory>

namespace icsneo {

class CANMessageCallback : public MessageCallback {
public:
	CANMessageCallback(fn_messageCallback cb, std::shared_ptr<CANMessageFilter> f) : MessageCallback(cb, f) {}
	CANMessageCallback(fn_messageCallback cb, CANMessageFilter f = CANMessageFilter()) : MessageCallback(cb, std::make_shared<CANMessageFilter>(f)) {}

	// Allow the filter to be placed first if the user wants (maybe in the case of a lambda)
	CANMessageCallback(std::shared_ptr<CANMessageFilter> f, fn_messageCallback cb) : MessageCallback(cb, f) {}
	CANMessageCallback(CANMessageFilter f, fn_messageCallback cb) : MessageCallback(cb, std::make_shared<CANMessageFilter>(f)) {}
};

};

#endif