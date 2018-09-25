#ifndef __NEOMESSAGE_H_
#define __NEOMESSAGE_H_

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 1)

typedef struct {
	uint64_t timestamp;
	const uint8_t* data;
	size_t length;
	char header[4];
	uint16_t netid;
	uint8_t type;
	char reserved[9];
} neomessage_t;

typedef struct {
	uint64_t timestamp;
	const uint8_t* data;
	size_t length;
	uint32_t arbid;
	uint16_t netid;
	uint8_t type;
	char reserved[9];
} neomessage_can_t;

#pragma pack(pop)

#ifdef __cplusplus
#include "communication/message/include/message.h"

namespace icsneo {
	neomessage_t CreateNeoMessage(const Message& message);
};
#endif

#endif