#ifndef __MFGCONFIGMESSAGE_H_
#define __MFGCONFIGMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/allmacaddressesmessage.h"
#include "icsneo/device/chipid.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace icsneo {

class MfgConfigMessage : public Message {
public:
	struct MfgDate {
		uint32_t day = 0;
		uint32_t month = 0;
		uint32_t year = 0;
	};

	struct VersionNumber {
		uint32_t major = 0;
		uint32_t minor = 0;
		std::optional<uint32_t> release;
		std::optional<uint32_t> build;
	};

	MfgConfigMessage() : Message(Message::Type::MfgConfig) {}

	static std::shared_ptr<MfgConfigMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static std::vector<uint8_t> EncodeArgumentsForGet();

	std::optional<uint64_t> serialNumber;
	std::optional<MfgDate> manufactureDate;
	std::optional<VersionNumber> hardwareRev;
	std::optional<uint8_t> productId;
	std::optional<VersionNumber> bootloaderRev;
	std::optional<std::string> usbDescriptor;
	std::vector<MACAddress> macAddresses;
	std::optional<std::string> pcbSerial;
	std::optional<ChipID> chipId;
	std::optional<uint64_t> imei;
	std::optional<uint8_t> parentProductId;
	std::optional<VersionNumber> softwareVersionLock;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __MFGCONFIGMESSAGE_H_
