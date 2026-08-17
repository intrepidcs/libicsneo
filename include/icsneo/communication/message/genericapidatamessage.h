#ifndef __GENERICAPIDATAMESSAGE_H_
#define __GENERICAPIDATAMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <vector>
#include <memory>

namespace icsneo {

class GenericAPIDataMessage : public Message {
public:
	static constexpr size_t GENERIC_API_BUFFER_SIZE = 513;

#pragma pack(push, 1)
	struct GenericAPIDataHeader {
		uint16_t bufferLength;
		uint8_t apiIndex;
		uint8_t instance;
		uint8_t functionId;
	};

	struct GenericAPIDataPacket
	{
		GenericAPIDataHeader header;
		uint8_t buffer[GENERIC_API_BUFFER_SIZE];
	};
#pragma pack(pop)

	static std::shared_ptr<GenericAPIDataMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	GenericAPIDataMessage() : Message(Type::GenericAPIData) {}

	uint8_t functionId;
	std::vector<uint8_t> buffer;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __GENERICAPIDATAMESSAGE_H_
