#ifndef _EXTENDED_COMPONENT_VERSIONS_RESPONSE_MESSAGE_H_
#define _EXTENDED_COMPONENT_VERSIONS_RESPONSE_MESSAGE_H_

#ifdef __cplusplus
#include "icsneo/communication/message/extendedresponsemessage.h"

namespace icsneo {
class ComponentVersion {
public:
	ComponentVersion(uint8_t valid, uint8_t componentInfo, uint32_t identifier, uint32_t dotVersion, uint32_t commitHash) :
		valid(valid), componentInfo(componentInfo), identifier(identifier), dotVersion(dotVersion), commitHash(commitHash) {}

	static ComponentVersion FromAppVersion(uint32_t identifier, uint8_t appMajor, uint8_t appMinor) {
		uint32_t dotVersion = (appMajor << 24) | (appMinor << 16);


		return ComponentVersion(
			static_cast<uint8_t>(1u),
			static_cast<uint8_t>(0u),
			identifier,
			dotVersion,
			static_cast<uint8_t>(0u)
		);
	}

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