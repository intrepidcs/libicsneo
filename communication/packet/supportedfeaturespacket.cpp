#include "icsneo/communication/packet/supportedfeaturespacket.h"
#include "icsneo/communication/message/supportedfeaturesmessage.h"

using namespace icsneo;

static constexpr uint16_t SupportedFeaturesCommandVersion = 1;
static constexpr size_t NumSupportedFeaturesFields =
	(static_cast<size_t>(SupportedFeature::numSupportedFeatures) + 31) / 32;
#pragma pack(push, 2)
struct SupportedFeaturesResponse {
	ExtendedResponseMessage::ResponseHeader header;
	uint16_t cmdVersion;
	uint16_t numValidBits;
	uint32_t featuresFields[NumSupportedFeaturesFields];
};
#pragma pack(pop)

std::shared_ptr<SupportedFeaturesMessage> SupportedFeaturesPacket::DecodeToMessage(const std::vector<uint8_t>& bytes) {
	auto msg = std::make_shared<SupportedFeaturesMessage>();
	// Length check: At least a header, a 2-byte cmdVersion field, and a 2-byte numValidBits field.
	if(bytes.size() < sizeof(ExtendedResponseMessage::ResponseHeader) + 4) {
		return msg; // Empty
	}
	// Get a reference to the payload to fully validate the length
	const auto& response = *reinterpret_cast<const SupportedFeaturesResponse*>(bytes.data());
	// Expected size is the header, cmdVersion and numValidBits fields, plus the number of 32-bit bitfields in the response based on numValidBits
	auto expectedSize = sizeof(ExtendedResponseMessage::ResponseHeader) + 4 + ((response.numValidBits + 31) / 32) * 4;
	// If the response is malformed (too small), return an empty message
	if(bytes.size() < expectedSize) {
		return msg; // Empty
	}
	unsigned int loopLimit = std::min<unsigned int>(response.numValidBits, NumSupportedFeaturesFields);
	for(unsigned int i = 0; i < loopLimit; ++i) {
		uint32_t wordOffset = i / 32;
		uint32_t bitOffset = i % 32;
		if((response.featuresFields[wordOffset] >> bitOffset) & 1) {
			msg->features.insert(static_cast<SupportedFeature>(i));
		}
	}
	return msg;
}
