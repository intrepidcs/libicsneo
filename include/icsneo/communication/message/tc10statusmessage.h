#ifndef __TC10STATUSMESSAGE_H
#define __TC10STATUSMESSAGE_H

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/tc10.h"

#include <memory>

namespace icsneo
{

class TC10StatusMessage : public Message {
public:
	static std::shared_ptr<TC10StatusMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	
	TC10StatusMessage(const TC10WakeStatus& wakeStatus, const TC10SleepStatus& sleepStatus) :
		Message(Type::TC10Status), wakeStatus(wakeStatus), sleepStatus(sleepStatus) {}
	
	const TC10WakeStatus wakeStatus;
	const TC10SleepStatus sleepStatus;
};

}

#endif // __cplusplus

#endif
