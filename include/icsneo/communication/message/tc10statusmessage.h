#ifndef __TC10STATUSMESSAGE_H
#define __TC10STATUSMESSAGE_H

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <memory>

namespace icsneo
{

enum class TC10WakeStatus : uint8_t {
	NoWakeReceived,
	WakeReceived,
};

enum class TC10SleepStatus : uint8_t {
	NoSleepReceived,
	SleepReceived,
	SleepFailed,
	SleepAborted,
};

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
