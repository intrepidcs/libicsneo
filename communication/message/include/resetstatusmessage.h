#ifndef __RESETSTATUSMESSAGE_H_
#define __RESETSTATUSMESSAGE_H_

#include "communication/message/include/main51message.h"
#include "communication/include/command.h"
#include <string>

namespace icsneo {

class ResetStatusMessage : public Message {
public:
	ResetStatusMessage() : Message() {}
	virtual ~ResetStatusMessage() = default;
	uint16_t mainLoopTime;
	uint16_t maxMainLoopTime;
	bool justReset;
	bool comEnabled;
	bool cmRunning;
	bool cmChecksumFailed;
	bool cmLicenseFailed;
	bool cmVersionMismatch;
	bool cmBootOff;
	bool hardwareFailure;
	bool usbComEnabled;
	bool linuxComEnabled;
	bool cmTooBig;
	bool hidUsbState;
	bool fpgaUsbState;
	uint16_t busVoltage;
	uint16_t deviceTemperature;
};

}

#endif