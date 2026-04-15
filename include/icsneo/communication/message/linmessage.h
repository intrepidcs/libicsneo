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
		LIN_ERROR = icsneoc2_lin_msg_type_error
	};

	static void calcChecksum(LINMessage& message);
	uint8_t calcProtectedID(uint8_t& id);

	LINMessage() {};
	LINMessage(uint8_t id) : ID(id & 0x3Fu), protectedID(calcProtectedID(ID)) {};

	uint8_t ID = 0;
	uint8_t protectedID = 0;
	uint8_t checksum = 0;
	LINMessage::Type linMsgType = Type::NOT_SET;
	bool isEnhancedChecksum = false;
	LINErrorFlags errFlags;
	LINStatusFlags statusFlags;
};

}

#endif // __cplusplus

#endif