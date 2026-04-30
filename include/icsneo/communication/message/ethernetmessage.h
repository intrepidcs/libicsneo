#ifndef __ETHERNETMESSAGE_H_
#define __ETHERNETMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <array>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <optional>

namespace icsneo {

using MACAddress = std::array<uint8_t, 6>;

class EthernetMessage : public Frame {
public:
	// Standard Ethernet fields
	// Frame Check Sequence
	std::optional<uint32_t> fcs;
	bool frameTooShort = false;
	bool noPadding = false;
	bool fcsVerified = false;
	bool txAborted = false;
	bool crcError = false;
	
	// T1S-specific fields
	struct T1S {
		T1S() {}

		bool isSymbol = false;
		bool isBurst = false;
		bool txCollision = false;
		bool isWake = false;
		uint8_t nodeId = 0;
		uint8_t burstCount = 0;
		uint8_t symbolType = 0;
	};
	std::optional<T1S> t1s;

	// TSN-specific fields
	// If we expand TSN we should probably do something similar to what we did above with T1S.
	// IEEE 802.1Qbu frame preemption
	std::optional<uint8_t> preemptionFlags;

	// Helper functions to extract Destination MAC from the data payload
	// returns std::nullopt if the data payload is not large enough
	std::optional<MACAddress> getDestinationMAC() const { 
		if(data.size() < 6) {
			return std::nullopt;
		}
		MACAddress mac;
		std::copy(data.begin(), data.begin() + 6, mac.begin());
		return mac;
	}

	// Helper functions to extract Source MAC from the data payload
	// returns std::nullopt if the data payload is not large enough
	std::optional<MACAddress> getSourceMAC() const { 
		if(data.size() < 12) {
			return std::nullopt;
		}
		MACAddress mac;
		std::copy(data.begin() + 6, data.begin() + 12, mac.begin());
		return mac;
	}

	// Helper function to extract EtherType from the data payload
	//
	// EtherType is a two-octet field in an Ethernet frame (big-endian).
	// It is used to indicate which protocol is encapsulated in the payload of the frame 
	// and is used at the receiving end by the data link layer to determine how the payload is processed.
	// For example, an EtherType of 0x0800 indicates that the payload is an IPv4 packet, while 0x86DD indicates an IPv6 packet.
	// 
	// returns std::nullopt if the data payload is not large enough
	std::optional<uint16_t> getEtherType() const { 
		if(data.size() < 14) {
			return std::nullopt;
		}
		// EtherType is stored in a 2-byte network byte order (big-endian)
		return static_cast<uint16_t>((uint16_t(data[12]) << 8) | uint16_t(data[13]));
	}
};

}

#endif // __cplusplus

#endif // __ETHERNETMESSAGE_H_
