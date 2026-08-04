#ifndef __SPIPORTKEYMESSAGE_H_
#define __SPIPORTKEYMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <array>
#include <memory>

namespace icsneo {

class SPIPortKeyMessage : public Message {
public:
	enum class Operation : uint8_t {
		WriteKey = 0,
		IsKeySet = 1,
		ClearKey = 2
	};

#pragma pack(push, 2)
	struct SPIPortKeyPacket
	{
		Operation op;
		uint8_t portIndex;
		bool isKeyLoaded;
		uint8_t rsvd;
		uint8_t key[16];
	};
#pragma pack(pop)

	static std::shared_ptr<SPIPortKeyMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	SPIPortKeyMessage() : Message(Type::SPIPortKeyOperation) {}

	bool isKeyLoaded;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __SPIPORTKEYMESSAGE_H_
