#ifndef __WIVIRESPONSEMESSAGE_H_
#define __WIVIRESPONSEMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/packet/wivicommandpacket.h"
#include <optional>
#include <string>

namespace icsneo {

namespace WiVI {

struct Info {
	uint8_t sleepRequest;
	uint16_t connectionTimeoutMinutes;
	std::vector<CaptureInfo> captures;
	uint8_t vinAvailable;
};

// The response for Command::WiVICommand
class ResponseMessage : public Message {
public:
	ResponseMessage() : Message(Message::Type::WiVICommandResponse) {}
	bool success = true;
	std::optional<Command> responseTo;
	std::optional<int32_t> value;
	std::optional<Info> info;
	std::optional<std::string> vin;
};

} // namespace WiVI

} // namespace icsneo

#endif // __cplusplus

#endif