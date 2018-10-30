#ifndef __READSETTINGSMESSAGE_H_
#define __READSETTINGSMESSAGE_H_

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/communication.h"

namespace icsneo {

class ReadSettingsMessage : public Message {
public:
	virtual ~ReadSettingsMessage() = default;
	
	enum class Response : uint8_t {
		OK = 0,
		GeneralFailure = 1,
		InvalidSubcommand = 2,
		InvalidSubversion = 3,
		NotEnoughMemory = 4,
		APIFailure = 5,
		APIUnsupported = 6
	};

	Response response;
};

}

#endif