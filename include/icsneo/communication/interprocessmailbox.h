#ifndef __INTERPROCESSMAILBOX_H_
#define __INTERPROCESSMAILBOX_H_

#ifdef __cplusplus

#include <cstdint>
#include "icsneo/platform/sharedmemory.h"
#include "icsneo/platform/sharedsemaphore.h"

static constexpr uint16_t MESSAGE_COUNT = 1024;
static constexpr uint16_t BLOCK_SIZE = 2048;
using LengthFieldType = uint16_t;
static constexpr uint8_t LENGTH_FIELD_SIZE = sizeof(LengthFieldType);
static constexpr uint16_t MAX_DATA_SIZE = BLOCK_SIZE - LENGTH_FIELD_SIZE;

namespace icsneo {

class InterprocessMailbox {
public:
	bool open(const std::string& name, bool create = false /* create the shared resources or not */);
	bool close();
	operator bool() const;

	// data must be large enough to hold at least MAX_DATA_SIZE
	// messageLength can be larger than MAX_DATA_SIZE if the message spans multiple blocks, only MAX_DATA_SIZE will be read
	bool read(void* data, LengthFieldType& messageLength, const std::chrono::milliseconds& timeout);

	// if messageLength is larger than MAX_DATA_SIZE it's expected that future write() calls will send the remaining data
	bool write(const void* data, LengthFieldType messageLength, const std::chrono::milliseconds& timeout);
private:
	icsneo::SharedSemaphore queuedSem;
	icsneo::SharedSemaphore emptySem;
	icsneo::SharedMemory sharedMem;
	unsigned index = 0; // index into messages;
	bool valid = false;
};

}

#endif // __cplusplus

#endif
