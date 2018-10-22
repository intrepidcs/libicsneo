#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include "icsneo/communication/network.h"
#include <vector>

namespace icsneo {

class Message {
public:
	virtual ~Message() = default;
	Network network;
	std::vector<uint8_t> data;
	uint64_t timestamp;
};

}

#endif