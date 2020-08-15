#ifndef __PACKET_H_
#define __PACKET_H_

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include <vector>
#include <stdint.h>

namespace icsneo {

class Packet {
public:
	Network network;
	std::vector<uint8_t> data;
};

}

#endif // __cplusplus

#endif