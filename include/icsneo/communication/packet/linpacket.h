#ifndef __LINPACKET_H__
#define __LINPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/linmessage.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/network.h"
#include <cstdint>
#include <memory>

namespace icsneo {

#pragma pack(push, 2)

struct HardwareLINPacket {
	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(LINMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
	struct {
		//CxLIN3
		uint16_t ErrRxOnlyBreak : 1;
		uint16_t ErrRxOnlyBreakSync : 1;
		uint16_t ID : 11;
		uint16_t NETWORKINDEX : 3;//DO NOT CLOBBER THIS

		// CxLIN
		uint8_t LINByte9;
		uint8_t ErrTxRxMismatch : 1;
		uint8_t TxChkSumEnhanced : 1;
		uint8_t TXCommander : 1;
		uint8_t TXResponder : 1;
		uint8_t ErrRxBreakNotZero : 1;
		uint8_t ErrRxBreakTooShort : 1;
		uint8_t ErrRxSyncNot55 : 1;
		uint8_t ErrRxDataGreater8 : 1;

		// CxLIN2
		uint16_t len : 4;
		uint16_t ExtendedNetworkIndexBit2 : 1;//DO NOT CLOBBER THIS
		uint16_t UpdateResponderOnce : 1;
		uint16_t HasUpdatedResponderOnce : 1;
		uint16_t ExtendedNetworkIndexBit : 1;//DO NOT CLOBBER THIS
		uint16_t BusRecovered : 1;
		uint16_t SyncFerr : 1; //Framing error in sync byte
		uint16_t MidFerr : 1; // Framing error in message id
		uint16_t ResponderByteFerr : 1; //Framing error in one of our responder bytes.
		uint16_t TxAborted : 1;//!< This transmit was aborted.
		uint16_t BreakOnly : 1;
		uint16_t : 2;
	} CoreMiniBitsLIN;

	uint8_t data[8];

	uint16_t stats; //CxTRB0STAT
	uint64_t timestamp; //Large timestamp
	//CoreMiniMsgExtendedHdr
	uint16_t networkID;
	uint16_t length;
};

#pragma pack(pop)

} //namespace libicsneo
#endif //_cplusplus
#endif