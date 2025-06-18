#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/livedata.h"

namespace icsneo
{

void LiveDataCommandMessage::appendSignalArg(LiveDataValueType valueType) {
	auto& arg = args.emplace_back(std::make_shared<LiveDataArgument>());
	arg->objectType = LiveDataObjectType::MISC;
	arg->objectIndex = 0u;
	arg->signalIndex = 0u;
	arg->valueType = valueType;
}

void LiveDataSetValueMessage::appendSetValue(LiveDataValueType valueType, const LiveDataValue& value) {
	auto& arg = args.emplace_back(std::make_shared<LiveDataArgument>());
	arg->objectType = LiveDataObjectType::MISC;
	arg->objectIndex = 0u;
	arg->signalIndex = 0u;
	arg->valueType = valueType;

	values.push_back(std::make_shared<LiveDataValue>(value));
}

} // namespace icsneo
