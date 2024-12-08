#ifndef __MAIN51MESSAGE_H_
#define __MAIN51MESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"

namespace icsneo {

class Main51Message : public InternalMessage {
public:
	Main51Message() : InternalMessage(Message::Type::Main51, _icsneo_netid_t::icsneo_netid_main51) {}
	Command command = Command(0);
	bool forceShortFormat = false; // Necessary for EnableNetworkCom and EnableNetworkComEx
};

}

#endif // __cplusplus

#endif