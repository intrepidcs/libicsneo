#ifndef __PACKET_H_
#define __PACKET_H_

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

#endif