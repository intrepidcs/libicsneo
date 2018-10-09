#ifndef __DECODER_H_
#define __DECODER_H_

#include "communication/message/include/message.h"
#include "communication/message/include/canmessage.h"
#include "communication/include/packet.h"
#include "communication/include/network.h"
#include <queue>
#include <vector>
#include <memory>

#pragma pack(push, 1)

namespace icsneo {

class Decoder {
public:
	static uint64_t GetUInt64FromLEBytes(uint8_t* bytes);
	std::shared_ptr<Message> decodePacket(const std::shared_ptr<Packet>& message);
	
private:
	typedef uint16_t icscm_bitfield;
	struct HardwareCANPacket {
		struct {
			icscm_bitfield IDE : 1;
			icscm_bitfield SRR : 1;
			icscm_bitfield SID : 11;
			icscm_bitfield EDL : 1;
			icscm_bitfield BRS : 1;
			icscm_bitfield ESI : 1;
		} header;
		struct {
			icscm_bitfield EID : 12;
			icscm_bitfield TXMSG : 1;
			icscm_bitfield TXAborted : 1;
			icscm_bitfield TXLostArb : 1;
			icscm_bitfield TXError : 1;
		} eid;
		struct {
			icscm_bitfield DLC : 4;
			icscm_bitfield RB0 : 1;
			icscm_bitfield IVRIF : 1;
			icscm_bitfield HVEnable : 1;// must be cleared before passing into CAN driver
			icscm_bitfield ExtendedNetworkIndexBit : 1;//DO NOT CLOBBER THIS
			icscm_bitfield RB1 : 1;
			icscm_bitfield RTR : 1;
			icscm_bitfield EID : 6;
		} dlc;
		unsigned char data[8];
		struct {
			uint64_t TS : 60;
			uint64_t : 3;
			uint64_t IsExtended : 1;
		} timestamp;
	};

	union CoreMiniStatusBits_t {
		struct {
			unsigned just_reset : 1;
			unsigned com_enabled : 1;
			unsigned cm_is_running : 1;
			unsigned cm_checksum_failed : 1;
			unsigned cm_license_failed : 1;
			unsigned cm_version_mismatch : 1;
			unsigned cm_boot_off : 1;
			unsigned hardware_failure : 1;//to check SRAM failure (for now)
			unsigned isPassiveConnect : 1;///< Always zero. Set to one when neoVI connection is passive,i.e. no async traffic
			unsigned usbComEnabled : 1;///<  Set to one when USB Host PC has enabled communication.
			unsigned linuxComEnabled : 1;///<  Set to one when Android (Linux) has enabled communication.
			unsigned cm_too_big : 1;
			unsigned hidUsbState : 1;
			unsigned fpgaUsbState : 1;
			unsigned reserved : 2;
		};
		uint32_t dword;
	};

	struct HardwareResetStatusPacket {
		uint16_t main_loop_time_25ns;
		uint16_t max_main_loop_time_25ns;
		CoreMiniStatusBits_t status;
		uint8_t histo[6];//!< Can hold histogram performance data.
		uint16_t spi1Kbps;//!< Spi1's kbps throughput.
		uint16_t initBits;//!< Bitfield with init states of drivers, 1 is succes, 0 is fail.
		uint16_t cpuMipsH;
		uint16_t cpuMipsL;
		uint16_t busVoltage;
		uint16_t deviceTemperature;
	} CoreMiniMsgResetStatus;

	// struct CoreMiniMsg {
	// 	CANMessage toCANMessage(Network netid);
	// 	union {
	// 		uint16_t CxTRB0SID16;
	// 		struct
	// 		{
	// 			icscm_bitfield IDE : 1;
	// 			icscm_bitfield SRR : 1;
	// 			icscm_bitfield SID : 11;
	// 			icscm_bitfield NETWORKINDEX : 3;//DO NOT CLOBBER THIS
	// 		} CxTRB0SID;
	// 		struct
	// 		{
	// 			icscm_bitfield : 13;
	// 			icscm_bitfield EDL : 1;
	// 			icscm_bitfield BRS : 1;
	// 			icscm_bitfield ESI : 1;
	// 		} CxTRB0FD;
	// 		struct
	// 		{
	// 			icscm_bitfield ErrRxOnlyBreak : 1;
	// 			icscm_bitfield ErrRxOnlyBreakSync : 1;
	// 			icscm_bitfield ID : 11;
	// 			icscm_bitfield NETWORKINDEX : 3;//DO NOT CLOBBER THIS
	// 		} CxLIN3;
	// 		struct
	// 		{
	// 			uint8_t D8;
	// 			uint8_t options : 4;
	// 			uint8_t TXMSG : 1;
	// 			uint8_t NETWORKINDEX : 3;//DO NOT CLOBBER THIS
	// 		} C1xJ1850;
	// 		struct
	// 		{
	// 			uint8_t D8;
	// 			uint8_t options : 4;
	// 			uint8_t TXMSG : 1;
	// 			uint8_t NETWORKINDEX : 3;//DO NOT CLOBBER THIS
	// 		} C1xISO;
	// 		struct
	// 		{
	// 			uint8_t D8;
	// 			uint8_t options : 4;
	// 			uint8_t TXMSG : 1;
	// 			uint8_t NETWORKINDEX : 3;//DO NOT CLOBBER THIS
	// 		} C1xJ1708;
	// 		struct
	// 		{
	// 			icscm_bitfield FCS_AVAIL : 1;
	// 			icscm_bitfield RUNT_FRAME : 1;
	// 			icscm_bitfield DISABLE_PADDING : 1;
	// 			icscm_bitfield PREEMPTION_ENABLED : 1;
	// 			icscm_bitfield MPACKET_TYPE : 4;
	// 			icscm_bitfield MPACKET_FRAG_CNT : 2;
	// 			icscm_bitfield : 6;
	// 		} C1xETH;
	// 		struct
	// 		{
	// 			uint16_t ID : 11;
	// 			uint16_t STARTUP : 1;
	// 			uint16_t SYNC : 1;
	// 			uint16_t NULL_FRAME : 1;
	// 			uint16_t PAYLOAD_PREAMBLE : 1;
	// 			uint16_t RESERVED_0 : 1;
	// 		} C1xFlex;
	// 		struct
	// 		{
	// 			uint8_t daqType;
	// 			uint8_t ethDaqRes1;
	// 		} C1xETHDAQ;
	// 	};
	// 	union {
	// 		uint16_t CxTRB0EID16;
	// 		struct
	// 		{
	// 			icscm_bitfield EID : 12;
	// 			icscm_bitfield TXMSG : 1;
	// 			icscm_bitfield TXAborted : 1;
	// 			icscm_bitfield TXLostArb : 1;
	// 			icscm_bitfield TXError : 1;
	// 		} CxTRB0EID;
	// 		struct
	// 		{
	// 			uint8_t LINByte9;
	// 			uint8_t ErrTxRxMismatch : 1;
	// 			uint8_t TxChkSumEnhanced : 1;
	// 			uint8_t TXMaster : 1;
	// 			uint8_t TXSlave : 1;
	// 			uint8_t ErrRxBreakNot0 : 1;
	// 			uint8_t ErrRxBreakTooShort : 1;
	// 			uint8_t ErrRxSyncNot55 : 1;
	// 			uint8_t ErrRxDataGreater8 : 1;
	// 		} CxLIN;
	// 		struct
	// 		{
	// 			uint8_t D9;
	// 			uint8_t D10;
	// 		} C2xJ1850;
	// 		struct
	// 		{
	// 			uint8_t D9;
	// 			uint8_t D10;
	// 		} C2xISO;
	// 		struct
	// 		{
	// 			uint8_t D9;
	// 			uint8_t D10;
	// 		} C2xJ1708;
	// 		struct
	// 		{
	// 			uint16_t txlen : 12;
	// 			uint16_t TXMSG : 1;
	// 			uint16_t : 3;
	// 		} C2xETH;
	// 		struct
	// 		{
	// 			uint16_t HDR_CRC_10 : 1;
	// 			uint16_t PAYLOAD_LEN : 7;
	// 			uint16_t RESERVED_1 : 4;
	// 			uint16_t TXMSG : 1;
	// 			uint16_t RESERVED_2 : 3;
	// 		} C2xFlex;
	// 	};
	// 	union {
	// 		// For use by CAN
	// 		uint16_t CxTRB0DLC16;
	// 		struct
	// 		{
	// 			icscm_bitfield DLC : 4;
	// 			icscm_bitfield RB0 : 1;
	// 			icscm_bitfield IVRIF : 1;
	// 			icscm_bitfield HVEnable : 1;// must be cleared before passing into CAN driver
	// 			icscm_bitfield ExtendedNetworkIndexBit : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield RB1 : 1;
	// 			icscm_bitfield RTR : 1;
	// 			icscm_bitfield EID : 6;
	// 		} CxTRB0DLC;
	// 		struct
	// 		{
	// 			icscm_bitfield len : 4;
	// 			icscm_bitfield ExtendedNetworkIndexBit2 : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield UpdateSlaveOnce : 1;
	// 			icscm_bitfield HasUpdatedSlaveOnce : 1;
	// 			icscm_bitfield ExtendedNetworkIndexBit : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield BusRecovered : 1;
	// 			icscm_bitfield SyncFerr : 1;//!< We got framing error in our sync byte.
	// 			icscm_bitfield MidFerr : 1;//!< We got framing error in our message id.
	// 			icscm_bitfield SlaveByteFerr : 1;//!< We got framing error in one of our slave bytes.
	// 			icscm_bitfield TxAborted : 1;//!< This transmit was aborted.
	// 			icscm_bitfield breakOnly : 1;
	// 			icscm_bitfield : 2;
	// 		} CxLIN2;
	// 		// For use by JVPW
	// 		struct
	// 		{
	// 			icscm_bitfield len : 4;
	// 			icscm_bitfield ExtendedNetworkIndexBit2 : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield just_tx_timestamp : 1;
	// 			icscm_bitfield first_seg : 1;
	// 			icscm_bitfield ExtendedNetworkIndexBit : 1;// do not clobber ExtendedNetworkIndexBit
	// 			icscm_bitfield D11 : 8;
	// 		} C3xJ1850;
	// 		// For use by the ISO/KEYWORD
	// 		struct
	// 		{
	// 			icscm_bitfield len : 4;
	// 			icscm_bitfield ExtendedNetworkIndexBit2 : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield FRM : 1;
	// 			icscm_bitfield INIT : 1;
	// 			icscm_bitfield ExtendedNetworkIndexBit : 1;// do not clobber ExtendedNetworkIndexBit
	// 			icscm_bitfield D11 : 8;
	// 		} C3xISO;
	// 		struct
	// 		{
	// 			icscm_bitfield len : 4;
	// 			icscm_bitfield ExtendedNetworkIndexBit2 : 1;//DO NOT CLOBBER THIS
	// 			icscm_bitfield FRM : 1;
	// 			icscm_bitfield : 1;
	// 			icscm_bitfield ExtendedNetworkIndexBit : 1;// do not clobber ExtendedNetworkIndexBit
	// 			icscm_bitfield pri : 8;
	// 		} C3xJ1708;
	// 		struct
	// 		{
	// 			uint16_t rsvd;
	// 		} C3xETH;
	// 		struct
	// 		{
	// 			uint16_t CYCLE : 6;
	// 			uint16_t HDR_CRC_9_0 : 10;
	// 		} C3xFlex;
	// 	};
	// 	unsigned char CxTRB0Dall[8];
	// 	union {
	// 		uint16_t CxTRB0STAT;
	// 		uint16_t J1850_TX_ID;
	// 	};
	// 	union {
	// 		struct
	// 		{
	// 			uint32_t uiTimeStamp10uS;
	// 			union {
	// 				uint32_t uiTimeStamp10uSMSB;
	// 				struct
	// 				{
	// 					unsigned : 28;
	// 					unsigned res_0s : 3;// must be 0!!!
	// 					unsigned bIsExtended : 1;// always 1 for CoreMiniMsgExtended.
	// 				};
	// 			};
	// 		};
	// 		int64_t uiTimeStampLarge;
	// 		uint8_t uiTimeStampBytes[8];
	// 	};
	// };
};

}

#pragma pack(pop)

#endif