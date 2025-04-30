#include "icsneo/communication/packet/componentversionpacket.h"
#include "icsneo/communication/message/componentversionsmessage.h"

using namespace icsneo;

#pragma pack(push, 2)
struct PackedComponentVersion {
	uint8_t valid;
	uint8_t expansionSlot;
	uint8_t componentInfo; // Component specific data (e.g. Linux: boot device)
	uint8_t reserved;
	uint32_t identifier;
	uint32_t dotVersion; // Represents a.b.c.d, a.b.c, or a.b, depending on leading zeros.
	uint32_t commitHash;
};

static constexpr size_t MaxReportedVersions = 16;
struct ComponentVersionsResponse {
	ExtendedResponseMessage::ResponseHeader header;
	uint16_t numVersions;
	PackedComponentVersion versions[MaxReportedVersions];
};
#pragma pack(pop)

std::shared_ptr<ComponentVersionsMessage> ComponentVersionPacket::DecodeToMessage(const std::vector<uint8_t>& bytes) {
	auto msg = std::make_shared<ComponentVersionsMessage>();
	// Length checks: At least a header and numVersions field.
	if(bytes.size() < sizeof(ExtendedResponseMessage::ResponseHeader) + 2) {
		return msg; // Empty
	}
	// Get a reference to the payload to fully validate the length
	const auto& response = *reinterpret_cast<const ComponentVersionsResponse*>(bytes.data());
	// Expected size is the header, numVersions field, and numVersions ComponentVersion objects.
	auto expectedSize = sizeof(ExtendedResponseMessage::ResponseHeader) + 2 + (response.numVersions * sizeof(PackedComponentVersion));
	// If the response is malformed (too small), return an empty message.
	if(bytes.size() < expectedSize) {
		return msg; // Empty
	}
	// Unpack into the portable class
	for(unsigned int i = 0; i < response.numVersions; ++i) {
		const auto& packedVersion = response.versions[i];
		msg->versions.emplace_back(
			packedVersion.valid,
			packedVersion.componentInfo,
			packedVersion.identifier,
			packedVersion.dotVersion,
			packedVersion.commitHash,
			packedVersion.expansionSlot
		);
	}
	return msg;
}
