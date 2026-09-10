#include "icsneo/communication/message/mfgconfigmessage.h"
#include "icsneo/communication/icspb.h"

#include <algorithm>

using namespace icsneo;

static MfgConfigMessage::VersionNumber versionFromProto(const settings::common::v1::VersionNumber& version) {
	MfgConfigMessage::VersionNumber decoded;
	decoded.major = version.major();
	decoded.minor = version.minor();
	if(version.has_release())
		decoded.release = version.release();
	if(version.has_build())
		decoded.build = version.build();
	return decoded;
}

std::shared_ptr<MfgConfigMessage> MfgConfigMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	MfgConfigMessage decoded;
	settings::manufacturing::v1::MfgConfig msg;

	if(!protoapi::processResponse(bytestream.data(), bytestream.size(), msg)) {
		return nullptr;
	}

	if(msg.has_serial_number()) {
		decoded.serialNumber = msg.serial_number();
	}
	if(msg.has_manufacture_date()) {
		const auto& date = msg.manufacture_date();
		decoded.manufactureDate = MfgDate{date.manufacture_day(), date.manufacture_month(), date.manufacture_year()};
	}
	if(msg.has_hardware_rev()) {
		decoded.hardwareRev = versionFromProto(msg.hardware_rev());
	}
	if(msg.has_product_id()) {
		decoded.productId = static_cast<uint8_t>(msg.product_id());
	}
	if(msg.has_bootloader_rev()) {
		decoded.bootloaderRev = versionFromProto(msg.bootloader_rev());
	}
	if(msg.has_usb_descriptor()) {
		decoded.usbDescriptor = msg.usb_descriptor();
	}
	for(const auto& mac : msg.mac_addresses()) {
		const auto& bytes = mac.mac_addr();
		if(bytes.size() < MACAddressLength)
			continue;
		MACAddress address{};
		std::copy_n(bytes.begin(), MACAddressLength, address.begin());
		decoded.macAddresses.push_back(address);
	}
	if(msg.has_pcb_serial()) {
		decoded.pcbSerial = msg.pcb_serial().pcb_serial();
	}
	if(msg.has_chip_id()) {
		decoded.chipId = static_cast<ChipID>(msg.chip_id());
	}
	if(msg.has_imei()) {
		decoded.imei = msg.imei();
	}
	if(msg.has_parent_product_id()) {
		decoded.parentProductId = static_cast<uint8_t>(msg.parent_product_id());
	}
	if(msg.has_software_version_lock()) {
		decoded.softwareVersionLock = versionFromProto(msg.software_version_lock());
	}

	return std::make_shared<MfgConfigMessage>(decoded);
}

std::vector<uint8_t> MfgConfigMessage::EncodeArgumentsForGet() {
	settings::manufacturing::v1::MfgConfig msg;
	msg.Clear();
	return protoapi::getPayload(protoapi::Command::GET, msg);
}
