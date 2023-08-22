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

} // namespace icsneo
