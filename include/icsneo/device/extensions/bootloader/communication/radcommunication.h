#ifndef __BOOTLOADER_RADCOMMUNICATION_H_
#define __BOOTLOADER_RADCOMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/communication.h"

#include <chrono>
#include <cstdint>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * The bootloader protocol used by RAD devices.
 *
 * Requests are a command byte followed by the payload, responses are the command byte,
 * a Response::Type byte and then the payload. Newer RAD bootloaders (the "Zynq binary
 * flasher") also prefix each request with ZynqFlasherSync and a 16-bit little-endian
 * length of the command byte plus payload.
 */
class RADCommunication : public Communication {
protected:
	static constexpr size_t MaxMessageLength = 1518;
	static constexpr uint8_t ZynqFlasherSync[4] = {0xAA, 0xAA, 0x55, 0x55};
	static constexpr size_t ZynqFlasherHeaderSize = sizeof(ZynqFlasherSync) + sizeof(uint16_t);

	struct Request {
		enum class Type : uint8_t {
			BootloaderVersion = 0x01,
			EnterApplication = 0x02,
			BeginTransfer = 0x03,
			FWPayload = 0x04,
			EndTransfer = 0x05,
			ProgramFlash = 0x06,
			DownloadedBinInfo = 0x07,
			ProgrammingStatus = 0x08,
			Authenticate = 0x09,
			SanityCheck = 0x0A,
			VerifyCRC = 0x0B,
			ValidateVersion = 0x0F,
			Invalid = 0xFF
		} type = Type::Invalid;

		uint8_t* payload = nullptr;
		size_t expectedResponseLen = 0;
	};

	Request makeRequest(Request::Type type, size_t payloadSize = 0, size_t expectedResponseLen = 0);

	struct Response {
		enum class Type {
			Failure = 0x00,
			Success = 0x01
		} type = Type::Failure;

		uint8_t* payload = nullptr;
		size_t size = 0;

		bool done = false;

		operator bool() const {
			return done && type == Type::Success;
		}
	};

	Response& sendRequest(const Request& request, std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
	bool sendRequestNoResponse();

	icsneo::Device::OpenStatusHandler logger; // Progress updates, set in flash()
	Response lastResponse; // Last response received from sendRequest
	bool supportsZynqBinaryFlasher = false; // Dictates how messages are formatted, this gets set in initialize()

	std::vector<uint8_t> rxBuffer;
	std::vector<uint8_t> txBuffer;
	Chip targetChip; // Chip to flash

	bool loadZips(const FWRepo& fw, std::vector<uint8_t>& chipSoftware);

	bool initialize();
	bool sanityCheck();
	bool getBootloaderVersion();
	bool validateChipVersion(const icsneo::DeviceAppVersion& srcVer);
	virtual bool authenticate();
	bool sendSoftware(const std::vector<uint8_t>& chipSoftware);
	bool getProgrammingStatus(size_t sizeToValidate);
	bool supportsFlashBuffer() const;
public:
	RADCommunication(icsneo::Device& dev, const ChipInfo& chipToFlash);

	bool enterApplication() override;
	bool finalize() override;
	bool flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) override;

	bool open() override;
	bool close() override;
	bool hasFinalizationStep() const override;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_RADCOMMUNICATION_H_
