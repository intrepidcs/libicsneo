#ifndef __LINMESSAGE_H_
#define __LINMESSAGE_H_

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
	static void calcChecksum(LINMessage& message);

	enum class Type {
		NOT_SET,
		LIN_COMMANDER_MSG,
		LIN_HEADER_ONLY,
		LIN_BREAK_ONLY,
		LIN_SYNC_ONLY,
		LIN_UPDATE_RESPONDER,
		LIN_ERROR
	};

	uint8_t ID = 0;
	uint8_t protectedID = 0;
	uint8_t checksum = 0;
	LINMessage::Type type = Type::NOT_SET;
	bool isEnhancedChecksum = false;
	bool isLINStd2x = true;
	LINErrorFlags errFlags;
	LINStatusFlags statusFlags;
};

}

#endif // __cplusplus

#endif