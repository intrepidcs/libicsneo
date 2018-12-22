#ifndef __ETHERNETMESSAGE_H_
#define __ETHERNETMESSAGE_H_

#include "icsneo/communication/message/message.h"

// Used for MACAddress.toString() only
#include <sstream>
#include <iomanip>

namespace icsneo {

struct MACAddress {
	uint8_t data[6];

	// Helpers
	std::string toString() const {
		std::stringstream ss;
		for(size_t i = 0; i < 6; i++) {
			ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
			if(i != 5)
				ss << ':';
		}
		return ss.str();
	}

	friend std::ostream& operator<<(std::ostream& os, const MACAddress& mac) {
		os << mac.toString();
		return os;
	}
};

class EthernetMessage : public Message {
public:
	bool preemptionEnabled = false;
	uint8_t preemptionFlags = 0;
	bool fcsAvailable = false;
	bool frameTooShort = false;
	bool noPadding = false;

	// Accessors
	const MACAddress& getDestinationMAC() const { return *(const MACAddress*)(data.data() + 0); }
	const MACAddress& getSourceMAC() const { return *(const MACAddress*)(data.data() + 6); }
	uint16_t getEtherType() const { return (data[12] << 8) | data[13]; }
};

}

#endif