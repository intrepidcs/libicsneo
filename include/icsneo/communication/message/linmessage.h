#ifndef __LINMESSAGE_H_
#define __LINMESSAGE_H_

#include "icsneo/icsneoc2messages.h"

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/network.h"
#include <vector>

namespace icsneo {

struct LINErrorFlags {
	bool ErrRxBreakOnly = false;
	bool ErrRxBreakSyncOnly = false;
	bool ErrTxRxMismatch = false;
	bool ErrRxBreakNotZero = false;
	bool ErrRxBreakTooShort = false;
	bool ErrRxSyncNot55 = false;
	bool ErrRxDataLenOver8 = false;
	bool ErrFrameSync = false;
	bool ErrFrameMessageID = false;
	bool ErrFrameResponderData = false;
	bool ErrChecksumMatch = false;
};

struct LINStatusFlags {
	bool TxChecksumEnhanced = false;
	bool TxCommander = false;
	bool TxResponder = false;
	bool TxAborted = false;
	bool UpdateResponderOnce = false;
	bool HasUpdatedResponderOnce = false;
	bool BusRecovered = false;
	bool BreakOnly = false;
	bool WakeupRequest = false;
};

class LINMessage : public Frame {
public:
	enum class Type : icsneoc2_lin_msg_type_t {
		NOT_SET = icsneoc2_lin_msg_type_not_set,
		LIN_COMMANDER_MSG = icsneoc2_lin_msg_type_commander_msg,
		LIN_HEADER_ONLY = icsneoc2_lin_msg_type_header_only,
		LIN_BREAK_ONLY = icsneoc2_lin_msg_type_break_only,
		LIN_SYNC_ONLY = icsneoc2_lin_msg_type_sync_only,
		LIN_UPDATE_RESPONDER = icsneoc2_lin_msg_type_update_responder,
		LIN_ERROR = icsneoc2_lin_msg_type_error,
		// needs firmware with LIN wakeup transmit; older firmware sends a commander header (break, sync, ID) instead
		LIN_WAKEUP_REQUEST = icsneoc2_lin_msg_type_wakeup_request
	};

	static void calcChecksum(LINMessage& message);
	uint8_t calcProtectedID(uint8_t& id);
	// wakeupPulseWidthUs in 50 us steps, 0 if it is outside 250 to 5000 us
	static int wakeupPulseWidthSteps(const LINMessage& message);

	LINMessage() {};
	LINMessage(uint8_t id) : ID(id & 0x3Fu), protectedID(calcProtectedID(ID)) {};

	uint8_t ID = 0;
	uint8_t protectedID = 0;
	uint8_t checksum = 0;
	LINMessage::Type linMsgType = Type::NOT_SET;
	bool isEnhancedChecksum = false;
	// LIN_WAKEUP_REQUEST only, the same as the Vehicle Spy LIN Send Wake Width property: 250 to 5000 us,
	// rounded to the nearest 50 us. Outside that range the transmit fails.
	uint16_t wakeupPulseWidthUs = 300;
	LINErrorFlags errFlags;
	LINStatusFlags statusFlags;
};

}

#endif // __cplusplus

#endif