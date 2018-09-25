#ifndef __CANMESSAGECALLBACK_H_
#define __CANMESSAGECALLBACK_H_

#include "communication/message/callback/include/messagecallback.h"
#include "communication/message/include/canmessage.h"
#include "communication/message/filter/include/canmessagefilter.h"
#include <memory>

namespace icsneo {

class CANMessageCallback : public MessageCallback {
public:
	CANMessageCallback(fn_messageCallback cb, CANMessageFilter f = CANMessageFilter()) : MessageCallback(cb, std::make_shared<CANMessageFilter>(f)) {}

	// Allow the filter to be placed first if the user wants (maybe in the case of a lambda)
	CANMessageCallback(CANMessageFilter f, fn_messageCallback cb) : MessageCallback(cb, std::make_shared<CANMessageFilter>(f)) {}
};

};

#endif