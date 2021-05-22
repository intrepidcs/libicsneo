#include "icsneo/communication/packet/versionpacket.h"

using namespace icsneo;

std::shared_ptr<VersionMessage> HardwareVersionPacket::DecodeMainToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < 3) // Not enough bytes to decode
		return std::shared_ptr<VersionMessage>();

	auto msg = std::make_shared<VersionMessage>(VersionMessage::MainChip);

	optional<DeviceAppVersion>& version = msg->Versions.emplace_back();
	version.emplace();
	version->major = bytestream[1];
	version->minor = bytestream[2];

	return msg;
}

std::shared_ptr<VersionMessage> HardwareVersionPacket::DecodeSecondaryToMessage(const std::vector<uint8_t>& bytestream) {
	auto msg = std::make_shared<VersionMessage>(VersionMessage::SecondaryChips);

	size_t bytesLeft = bytestream.size();
	if(bytesLeft)
		bytesLeft--; // Disregard command byte
	while(bytesLeft >= 3) {
		const bool versionValid = bytestream[bytestream.size() - bytesLeft + 0];
		optional<DeviceAppVersion>& version = msg->Versions.emplace_back();
		if(versionValid) {
			version.emplace();
			version->major = bytestream[bytestream.size() - bytesLeft + 1];
			version->minor = bytestream[bytestream.size() - bytesLeft + 2];
		}
		bytesLeft -= std::min<size_t>(3, bytesLeft);
	}

	return msg;
}
