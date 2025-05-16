#ifndef __LOGDATAMESSAGE_H_
#define __LOGDATAMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <cstdint>
#include <memory>

namespace icsneo {

class LogDataMessage : public RawMessage {
public:
	static std::shared_ptr<LogDataMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	LogDataMessage(std::wstring logDataString) :
		RawMessage(Message::Type::LogData, Network::NetID::Data_To_Host), logMessage(logDataString) {}
	
    std::wstring logMessage;
};

}

#endif // __cplusplus

#endif