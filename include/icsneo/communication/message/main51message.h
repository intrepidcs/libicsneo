#ifndef __MAIN51MESSAGE_H_
#define __MAIN51MESSAGE_H_

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/communication.h"

namespace icsneo {

class Main51Message : public Message {
public:
	virtual ~Main51Message() = default;
	Command command;
};

}

#endif