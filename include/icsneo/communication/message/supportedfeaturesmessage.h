#ifndef _SUPPORTED_FEATURES_RESPONSE_MESSAGE_H_
#define _SUPPORTED_FEATURES_RESPONSE_MESSAGE_H_

#ifdef __cplusplus
#include "icsneo/communication/message/extendedresponsemessage.h"
#include <cstdint>
#include <set>

namespace icsneo {

enum class SupportedFeature : uint16_t {
	networkDWCAN01 = 0,
	networkDWCAN02 = 1,
	networkDWCAN03 = 2,
	networkDWCAN04 = 3,
	networkDWCAN05 = 4,
	networkDWCAN06 = 5,
	networkDWCAN07 = 6,
	networkDWCAN08 = 7,
	networkTerminationDWCAN01 = 8,
	networkTerminationDWCAN02 = 9,
	networkTerminationDWCAN03 = 10,
	networkTerminationDWCAN04 = 11,
	networkTerminationDWCAN05 = 12,
	networkTerminationDWCAN06 = 13,
	networkTerminationDWCAN07 = 14,
	networkTerminationDWCAN08 = 15,
	enhancedFlashDriver = 16,
	rtcCalibration = 17,
	rtcClosedLoopCalibration = 18,
	numSupportedFeatures,
};

class SupportedFeaturesMessage : public Message {
public:
	SupportedFeaturesMessage() : Message(Message::Type::SupportedFeatures) {}
	std::set<SupportedFeature> features;
};

}

#endif // __cplusplus

#endif // _SUPPORTED_FEATURES_RESPONSE_MESSAGE_H_