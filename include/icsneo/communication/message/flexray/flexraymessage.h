#ifndef __FLEXRAYMESSAGE_H_
#define __FLEXRAYMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/device/extensions/flexray/symbol.h"
#include "icsneo/device/extensions/flexray/crcstatus.h"
#include "icsneo/device/extensions/flexray/channel.h"

namespace icsneo {

class FlexRayMessage : public Message {
public:
	uint16_t slotid = 0;
	double tsslen = 0;
	double framelen = 0;
	FlexRay::Symbol symbol = FlexRay::Symbol::None;
	FlexRay::CRCStatus headerCRCStatus = FlexRay::CRCStatus::OK;
	uint16_t headerCRC = 0;
	FlexRay::CRCStatus crcStatus = FlexRay::CRCStatus::OK;
	uint32_t frameCRC = 0;
	FlexRay::Channel channel = FlexRay::Channel::None;
	bool nullFrame = false;
	bool reserved0was1 = false;
	bool payloadPreamble = false;
	bool sync = false;
	bool startup = false;
	bool dynamic = false;
	uint8_t cycle = 0; // baseCycle when transmitting
	uint8_t cycleRepetition = 0; // only for transmit
};

}

#endif // __cplusplus

#endif