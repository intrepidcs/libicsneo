#ifndef __EXTENDEDDATAMESSAGE_H_
#define __EXTENDEDDATAMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"


namespace icsneo {

class ExtendedDataMessage : public RawMessage {
public:
	#pragma pack(push, 2)
	struct ExtendedDataHeader {
		ExtendedDataSubCommand subCommand;
		uint32_t userValue;
		uint32_t offset;
		uint32_t length;
	};
	#pragma pack(pop)
	
	static constexpr size_t MaxExtendedDataBufferSize = 2048;
	const ExtendedDataHeader header;

	ExtendedDataMessage(ExtendedDataHeader params) : RawMessage(Message::Type::RawMessage, Network::NetID::ExtendedData), header{params} {}

};

}

#endif // __cplusplus

#endif