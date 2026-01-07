#ifndef __ETHERNETMESSAGE_H_
#define __ETHERNETMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace icsneo {

struct MACAddress {
	uint8_t data[6];

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

class EthernetMessage : public Frame {
public:
	// Standard Ethernet fields
	bool preemptionEnabled = false;
	uint8_t preemptionFlags = 0;
	std::optional<uint32_t> fcs;
	bool frameTooShort = false;
	bool noPadding = false;
	bool fcsVerified = false;
	bool txAborted = false;
	bool crcError = false;
	bool isT1S = false;

	
	bool isT1SSymbol = false;
	bool isT1SBurst = false;
	bool txCollision = false;
	bool isT1SWake = false;
	uint8_t t1sNodeId = 0;
	uint8_t t1sBurstCount = 0;
	uint8_t t1sSymbolType = 0;

	// Accessors
	const MACAddress& getDestinationMAC() const { return *(const MACAddress*)(data.data() + 0); }
	const MACAddress& getSourceMAC() const { return *(const MACAddress*)(data.data() + 6); }
	uint16_t getEtherType() const { return (data[12] << 8) | data[13]; }
};

}

#endif // __cplusplus

#endif