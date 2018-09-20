#ifndef __NEOMESSAGE_H_
#define __NEOMESSAGE_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint16_t netid;
	uint8_t type;
	char header[4];
	const uint8_t* data;
	size_t length;
	uint64_t timestamp;
	char reserved[8];
} neomessage_t;

typedef struct {
	uint16_t netid;
	uint8_t type;
	uint32_t arbid;
	const uint8_t* data;
	size_t length;
	uint64_t timestamp;
	char reserved[8];
} neomessage_can_t;

#ifdef __cplusplus
#include "communication/message/include/message.h"

namespace icsneo {
	neomessage_t CreateNeoMessage(const Message& message);
};
#endif

#endif