#ifndef __RESETSTATUSMESSAGE_H_
#define __RESETSTATUSMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/main51message.h"
#include "icsneo/communication/command.h"
#include <optional>
#include <string>

namespace icsneo {

class ResetStatusMessage : public Message {
public:
	ResetStatusMessage() : Message(Message::Type::ResetStatus) {}

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
	std::optional<uint16_t> busVoltage;
	std::optional<uint16_t> deviceTemperature;
};

}

#endif // __cplusplus

#endif