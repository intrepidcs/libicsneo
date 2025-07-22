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
	ExtendedResponseFilter(icsneo::ExtendedCommand cmd) : MessageFilter(Message::Type::ExtendedResponse), command(cmd), response(std::nullopt) {}
	ExtendedResponseFilter(icsneo::ExtendedResponse resp) : MessageFilter(Message::Type::ExtendedResponse), command(std::nullopt), response(resp) {}
	ExtendedResponseFilter(icsneo::ExtendedCommand cmd, icsneo::ExtendedResponse resp) 
		: MessageFilter(Message::Type::ExtendedResponse), command(cmd), response(resp) {}

	bool match(const std::shared_ptr<Message>& message) const override {
		if(!MessageFilter::match(message)) {
			return false;
		}
		const auto respMsg = std::static_pointer_cast<ExtendedResponseMessage>(message);
		return respMsg && matchResponse(respMsg->command, respMsg->response);
	}

private:
	std::optional<icsneo::ExtendedCommand> command = std::nullopt;
	std::optional<icsneo::ExtendedResponse> response = std::nullopt;

	bool matchResponse(icsneo::ExtendedCommand cmd, icsneo::ExtendedResponse resp) const {
		if(command) {
			if(*command != cmd) {
				return false;
			}
		}
		if(response) {
			if(*response != resp) {
				return false;
			}
		}

		return true;
	}
};

}

#endif // __cplusplus

#endif