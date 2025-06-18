#ifndef __EXTENDEDLIVEDATAMESSAGE_H_
#define __EXTENDEDLIVEDATAMESSAGE_H_
#ifdef __cplusplus

#include <chrono>
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/livedata.h"

namespace icsneo {
class LiveDataMessage : public RawMessage {
public:
	LiveDataMessage() : RawMessage(Message::Type::LiveData, Network::NetID::ExtendedCommand) {}
	LiveDataHandle handle;
	LiveDataCommand cmd;
};

class LiveDataCommandMessage : public LiveDataMessage {
public:
	LiveDataCommandMessage() {}
	std::chrono::milliseconds updatePeriod;
	std::chrono::milliseconds expirationTime;
	std::vector<std::shared_ptr<LiveDataArgument>> args;
	void appendSignalArg(LiveDataValueType valueType);
};

class LiveDataValueMessage : public LiveDataMessage {
public:
	LiveDataValueMessage() {}
	uint32_t numArgs;
	std::vector<std::shared_ptr<LiveDataValue>> values;
};

class LiveDataStatusMessage : public LiveDataMessage {
public:
	LiveDataStatusMessage() {}
	LiveDataCommand requestedCommand;
	LiveDataStatus status;
};

class LiveDataSetValueMessage : public LiveDataMessage {
public:
	LiveDataSetValueMessage() {}
	std::vector<std::shared_ptr<LiveDataArgument>> args;
	std::vector<std::shared_ptr<LiveDataValue>> values;
	void appendSetValue(LiveDataValueType valueType, const LiveDataValue& value);
};

} // namespace icsneo

#endif // __cplusplus
#endif // __EXTENDEDLIVEDATAMESSAGE_H_