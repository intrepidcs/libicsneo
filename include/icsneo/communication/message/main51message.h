#ifndef __MAIN51MESSAGE_H_
#define __MAIN51MESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"

namespace icsneo {

class Main51Message : public Message {
public:
	virtual ~Main51Message() = default;
	Command command = Command(0);
	bool forceShortFormat = false; // Necessary for EnableNetworkCom and EnableNetworkComEx
};

}

#endif // __cplusplus

#endif