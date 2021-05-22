#ifndef __ISO9141MESSAGE_H_
#define __ISO9141MESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <array>

namespace icsneo {

class ISO9141Message : public Frame {
public:
	std::array<uint8_t, 3> header;
	bool isInit = false;
	bool isBreak = false;
	bool framingError = false;
	bool overflowError = false;
	bool parityError = false;
	bool rxTimeoutError = false;
	// Checksum not yet implemted, it's just at the end of the data if enabled for now
	// bool checksumError = false;
	// bool hasChecksum = false;
	// uint8_t checksum = 0;
};

}

#endif // __cplusplus

#endif