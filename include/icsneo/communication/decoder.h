#ifndef __DECODER_H_
#define __DECODER_H_

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/packet.h"
#include "icsneo/communication/network.h"
#include "icsneo/api/errormanager.h"
#include <queue>
#include <vector>
#include <memory>

namespace icsneo {

class Decoder {
public:
	static uint64_t GetUInt64FromLEBytes(uint8_t* bytes);

	Decoder(device_errorhandler_t err) : err(err) {}
	bool decode(std::shared_ptr<Message>& result, const std::shared_ptr<Packet>& packet);

	uint16_t timestampResolution = 25;

private:
	device_errorhandler_t err;

#pragma pack(push, 1)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif
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
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
	};
#pragma pack(pop)
};

}

#endif