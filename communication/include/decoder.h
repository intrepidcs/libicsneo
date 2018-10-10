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
			icscm_bitfield EID2 : 6;
		} dlc;
		unsigned char data[8];
		uint16_t stats;
		struct {
			uint64_t TS : 60;
			uint64_t : 3; // Reserved for future status bits
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
};

}

#pragma pack(pop)

#endif