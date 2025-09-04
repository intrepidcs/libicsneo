#ifndef __FLEXRAYMESSAGEBUFFER_H_
#define __FLEXRAYMESSAGEBUFFER_H_

#ifdef __cplusplus

#include <cstdint>

namespace icsneo {

namespace FlexRay {

#define INIT(x) = x

#define neoflexray_message_buffer_t icsneo::FlexRay::MessageBuffer
struct MessageBuffer {
#else // __cplusplus

#include <stdbool.h>
#include <stdint.h>

#define INIT(x)

typedef struct {

#endif // __cplusplus

	bool isDynamic INIT(false);
	bool isSync INIT(false); // Must be set if isStartup is set!
	bool isStartup INIT(false);
	bool isNetworkManagementFrame INIT(false);
	bool isTransmit INIT(false);
	uint16_t frameID INIT(0); // Frame/Slot ID
	bool channelA INIT(false);
	bool channelB INIT(false);
	uint8_t frameLengthBytes INIT(0);
	uint8_t baseCycle INIT(0);
	uint8_t cycleRepetition INIT(0);
	bool continuousMode INIT(false); // If false, mode is continuous mode
	uint16_t _dataPointer INIT(0); // For internal use
	uint16_t _id INIT(0); // For internal use

#ifndef __cplusplus
} neoflexray_message_buffer_t;
#else
};

} // namespace FlexRay

} // namespace icsneo
#endif // __cplusplus

#endif // __FLEXRAYMESSAGEBUFFER_H_