#ifndef __MAIN51MESSAGEFILTER_H_
#define __MAIN51MESSAGEFILTER_H_

#ifdef __cplusplus

#include "icsneo/communication/message/filter/messagefilter.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/message/main51message.h"
#include <memory>
#include <iostream>

namespace icsneo {

class Main51MessageFilter : public MessageFilter {
public:
	Main51MessageFilter() : MessageFilter(Message::Type::Main51), command(INVALID_COMMAND) {}
	// Don't filter on Type::Main51 for Command as some Commands have their own type
	// We still guarantee it's a Main51Message if it matches because of the dynamic_pointer_cast below
	Main51MessageFilter(Command command) : command(command) { includeInternalInAny = true; }

	bool match(const std::shared_ptr<Message>& message) const {
		if(!MessageFilter::match(message)) {
			//std::cout << "message filter did not match base for " << message->network << std::endl;
			return false;
		}
		const auto main51Message = std::dynamic_pointer_cast<Main51Message>(message);
		return main51Message && matchCommand(main51Message->command);
	}

private:
	static constexpr Command INVALID_COMMAND = (Command)0xff;
	Command command;
	bool matchCommand(Command mcommand) const {
		if(command == INVALID_COMMAND)
			return true;
		return command == mcommand;
	}
};

}

#endif // __cplusplus

#endif