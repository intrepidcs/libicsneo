#ifndef __EXTENDEDRESPONSEFILTER_H_
#define __EXTENDEDRESPONSEFILTER_H_

#ifdef __cplusplus

#include "icsneo/communication/message/filter/messagefilter.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include <memory>
#include <iostream>

namespace icsneo {

class ExtendedResponseFilter : public MessageFilter {
public:
	ExtendedResponseFilter(icsneo::ExtendedResponse resp) : MessageFilter(Message::Type::ExtendedResponse), response(resp) {}

	bool match(const std::shared_ptr<Message>& message) const override {
		if(!MessageFilter::match(message)) {
			return false;
		}
		const auto respMsg = std::static_pointer_cast<ExtendedResponseMessage>(message);
		return respMsg && matchResponse(respMsg->response);
	}

private:
	icsneo::ExtendedResponse response;
	bool matchResponse(icsneo::ExtendedResponse resp) const {
		return response == resp;
	}
};

}

#endif // __cplusplus

#endif