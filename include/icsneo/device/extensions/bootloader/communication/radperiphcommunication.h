#ifndef __BOOTLOADER_RADPERIPHCOMMUNICATION_H_
#define __BOOTLOADER_RADPERIPHCOMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/communication.h"
#include "icsneo/device/extensions/bootloader/communication/bootloaderversion.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"

#include <chrono>
#include <cstdint>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * The bootloader protocol used by peripheral chips on the RAD-Galaxy 2, tunneled through
 * Command::PeripheralBootloader. Requests are the chip id, the Request and the arguments.
 * Replies are a byte, ReplySuccess, then for bootloader requests ReplyAccepted, then the payload.
 */
class RADPeriphCommunication : public Communication {
protected:
	static constexpr size_t ReplySuccessOffset = 1;
	static constexpr size_t ReplyAcceptedOffset = 2;
	static constexpr uint8_t ReplySuccess = 0x01;
	static constexpr uint8_t ReplyAccepted = 0x77;
	static constexpr size_t ReplyHeaderSize = 3;
	// Requests that write to the bootloader have three leading zero bytes before their arguments
	static constexpr size_t BootloaderArgsOffset = 3;
	// Flash sectors are sent as two requests, the first with this many bytes and the second with the rest
	static constexpr size_t FlashSectorFirstChunkSize = 1024;

	enum class Mode : uint8_t {
		NoAnswer = 0x00u,
		MainApplication = 0xAFu,
		Bootloader = 0xFEu
	} mode = Mode::NoAnswer;

	Chip targetChip;
	uint32_t serialNumber = 0;
public:
	enum class Request : uint8_t {
		SwitchToApp = 0x01u,
		SwitchToBootloader = 0x02u,
		Version = 0x12u,
		Authenticate = 0x13u,
		FlashSector = 0x14u,
		FirmwareValidate = 0x16u,
		Erase = 0x20u,
		Echo = 0xAFu,
		Mode = 0xADu,
	};

	struct Response {
		bool success = false;
		std::vector<uint8_t> data;

		operator bool() const {
			return success;
		}
	};

	bool waitForConnect(std::chrono::milliseconds timeout = std::chrono::milliseconds(30000));

	bool sendRequestNoResponse(Request req, std::vector<uint8_t>&& args = {});
	bool sendRequestNoResponse(Request req, ChipID id, std::vector<uint8_t>&& args = {});
	Response sendRequest(Request req, std::vector<uint8_t>&& args = {}, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
	Response sendRequest(Request req, ChipID id, std::vector<uint8_t>&& args = {}, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

	Mode getMode();
	bool sendPeriphTest();
	bool readSerialNumber();
	bool authenticate();
	bool validateFirmware();
	bool flashSector(const IEF::Sector& sector);

	RADPeriphCommunication(icsneo::Device& dev, const ChipInfo& chipToFlash);
	~RADPeriphCommunication();

	bool flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) override;
	bool open() override;
	bool close() override;
	bool enterApplication() override;
	bool finalize() override;
	bool hasFinalizationStep() const override;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_RADPERIPHCOMMUNICATION_H_
