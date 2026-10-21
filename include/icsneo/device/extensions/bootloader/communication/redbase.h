#ifndef __BOOTLOADER_REDBASE_H_
#define __BOOTLOADER_REDBASE_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/communication.h"
#include "icsneo/device/extensions/bootloader/communication/redpacketizer.h"

#include <chrono>
#include <cstdint>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * Implementation shared by REDCommunication and REDCoreCommunication
 *
 * Requests are framed as:
 *   uint8_t  REDPacketizer::Magic
 *   uint16_t chip id (little-endian)
 *   uint16_t length of the command byte, arguments and padding (little-endian)
 *   uint8_t  command
 *   uint8_t  arguments[]
 *   uint8_t  optional PaddingByte, see sendCommandRaw()
 */
class REDCommunicationBase : public Communication {
public:
	REDCommunicationBase(icsneo::Device& dev, const ChipInfo& chip);
	~REDCommunicationBase() {
		// This must happen before the Packetizer is destroyed
		device.com->resumeReads();
	}

	bool open() override;
	bool close() override;
	bool hasFinalizationStep() const override {
		return false;
	}
	bool finalize() override {
		return true;
	}

protected:
	static constexpr uint8_t PaddingByte = 0x55;

	Chip targetChip;

	std::vector<REDPacketizer::RxFrame> lastData;
	REDPacketizer rp;

	bool handleData();
	bool sendCommand(uint8_t cmd, std::chrono::seconds timeout, bool clearLastResponse = true) { return sendCommand(cmd, {}, timeout, clearLastResponse); }
	bool sendCommand(uint8_t cmd, std::vector<uint8_t> args = {}, std::chrono::seconds timeout = std::chrono::seconds(1), bool clearLastResponse = true);
	virtual bool sendCommandRaw(uint8_t cmd, std::vector<uint8_t> args = {});
	bool waitForRx(std::chrono::milliseconds timeout);
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_REDBASE_H_
