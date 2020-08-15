#ifndef __SERIALNUMBERMESSAGE_H_
#define __SERIALNUMBERMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/main51message.h"
#include "icsneo/communication/command.h"
#include <string>

namespace icsneo {

// The response for Command::RequestSerialNumber
class SerialNumberMessage : public Main51Message {
public:
	SerialNumberMessage() : Main51Message() { command = Command::RequestSerialNumber; }
	virtual ~SerialNumberMessage() = default;
	std::string deviceSerial;
	uint8_t macAddress[6]; // This might be all zeros even if `hasMacAddress` is true
	bool hasMacAddress = false; // The message might not actually be long enough to contain a MAC address, in which case we mark this
	uint8_t pcbSerial[16];
	bool hasPCBSerial = false;
};

}

#endif // __cplusplus

#endif