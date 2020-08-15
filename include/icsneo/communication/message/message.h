#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include <vector>

namespace icsneo {

class Message {
public:
	virtual ~Message() = default;
	Network network;
	std::vector<uint8_t> data;
	uint64_t timestamp = 0;
	uint16_t description = 0;
	bool transmitted = false;
	bool error = false;
};

}

#endif // __cplusplus

#endif