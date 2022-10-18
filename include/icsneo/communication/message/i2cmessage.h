#ifndef __I2CMESSAGE_H_
#define __I2CMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <vector>

namespace icsneo {

class I2CMessage : public Frame {
public:
	enum class DeviceMode : uint8_t {
		Target = 0,
		Controller = 1
	};

	enum class Direction : uint8_t {
		Write = 0,
		Read = 1
	};

	bool isExtendedID = false;
	bool isTXMsg = false;
	bool txError = false;
	bool txLostArb = false;
	bool txAborted = false;
	bool txNack = false;
	bool txTimeout = false;
	uint16_t stats = static_cast<uint16_t>(0x0000u);
	uint16_t address;
	Direction direction;
	DeviceMode deviceMode;

	//Must contain the target register address to read or write
	std::vector<uint8_t> controlBytes;

	//The device expects a dataBytes payload even if you're reading
	//In the case of a read these bytes aren't interesting, but they have to be there
	//Add bytes to write, or the same number of junk bytes you expect the device send back
	std::vector<uint8_t> dataBytes;
};

}

#endif // __cplusplus

#endif
