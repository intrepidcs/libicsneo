#ifndef _EXTENDED_COMPONENT_VERSIONS_RESPONSE_MESSAGE_H_
#define _EXTENDED_COMPONENT_VERSIONS_RESPONSE_MESSAGE_H_

#ifdef __cplusplus
#include "icsneo/communication/message/extendedresponsemessage.h"

namespace icsneo {
class ComponentVersion {
public:
	ComponentVersion(uint8_t valid, uint8_t componentInfo, uint32_t identifier, uint32_t dotVersion, uint32_t commitHash) :
		valid(valid), componentInfo(componentInfo), identifier(identifier), dotVersion(dotVersion), commitHash(commitHash) {}
	const bool valid;
	const uint8_t componentInfo;
	const uint32_t identifier;
	const uint32_t dotVersion;
	const uint32_t commitHash;
};

class ComponentVersionsMessage : public Message {
public:
	ComponentVersionsMessage() : Message(Message::Type::ComponentVersions) {}
	std::vector<ComponentVersion> versions;
};

}

#endif // __cplusplus

#endif // _EXTENDED_COMPONENT_VERSIONS_RESPONSE_MESSAGE_H_