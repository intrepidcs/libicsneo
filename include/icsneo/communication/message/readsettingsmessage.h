#ifndef __READSETTINGSMESSAGE_H_
#define __READSETTINGSMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/communication.h"

namespace icsneo {

class ReadSettingsMessage : public InternalMessage {
public:
	ReadSettingsMessage() : InternalMessage(Message::Type::ReadSettings, Network::_icsneo_netid_t::icsneo_netid_read_settings) {}
	
	enum class Response : uint8_t {
		OK = 0,
		GeneralFailure = 1,
		InvalidSubcommand = 2,
		InvalidSubversion = 3,
		NotEnoughMemory = 4,
		APIFailure = 5,
		APIUnsupported = 6,
		OKDefaultsUsed = 7, // Got the settings okay, but the defaults were used (after firmware upgrade or a checksum error)
	};

	Response response;
};

}

#endif // __cplusplus

#endif