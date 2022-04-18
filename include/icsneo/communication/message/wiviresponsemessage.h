#ifndef __WIVIRESPONSEMESSAGE_H_
#define __WIVIRESPONSEMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"
#include "icsneo/platform/optional.h"
#include "icsneo/communication/packet/wivicommandpacket.h"
#include <string>

namespace icsneo {

namespace WiVI {

struct Info {
	uint8_t sleepRequest;
	uint16_t connectionTimeoutMinutes;
	std::vector<CaptureInfo> captures;
};

// The response for Command::WiVICommand
class ResponseMessage : public Message {
public:
	ResponseMessage() : Message(Message::Type::WiVICommandResponse) {}
	bool success = true;
	optional<Command> responseTo;
	optional<int32_t> value;
	optional<Info> info;
};

} // namespace WiVI

} // namespace icsneo

#endif // __cplusplus

#endif