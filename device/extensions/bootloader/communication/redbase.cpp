#include "icsneo/device/extensions/bootloader/communication/redbase.h"

#include <limits>

namespace icsneo {

namespace Bootloader {

REDCommunicationBase::REDCommunicationBase(icsneo::Device& dev, const ChipInfo& chip)
	: Communication(dev), targetChip(chip) {}

bool REDCommunicationBase::open() {
	device.com->pauseReads();
	return true;
}

bool REDCommunicationBase::close() {
	device.com->resumeReads();
	return true;
}

bool REDCommunicationBase::handleData() {
	auto& rb = device.com->driver->getReadBuffer();

	if(rp.input(rb)) {
		lastData = rp.output();
		return true;
	}

	return false;
}

bool REDCommunicationBase::waitForRx(std::chrono::milliseconds timeout) {
	return device.com->driver->waitForRx([this]() { return handleData(); }, timeout);
}

bool REDCommunicationBase::sendCommand(uint8_t cmd, std::vector<uint8_t> args, std::chrono::seconds timeout, bool clearLastResponse) {
	lastData.clear();
	device.com->driver->getReadBuffer().clear();

	if(!sendCommandRaw(cmd, std::move(args)))
		return false;

	if(!waitForRx(std::chrono::duration_cast<std::chrono::milliseconds>(timeout)))
		return false;

	const bool ret = lastData.front().resp == REDPacketizer::Response::Accepted;

	if(clearLastResponse) {
		lastData.clear();
	}
	return ret;
}

bool REDCommunicationBase::sendCommandRaw(uint8_t cmd, std::vector<uint8_t> args) {
	// For some reason, some devices need it padded to even total bytes, and some to odd total bytes
	const bool padEven = device.getType() == DeviceType::FIRE2;

	args.insert(args.begin(), uint8_t(cmd)); // Prepend the command byte to the arguments
	if(args.size() > std::numeric_limits<uint16_t>::max())
		return false;

	uint16_t size = uint16_t(args.size());
	// The header counts, and the header is 5 bytes, so we're checking for even inner size here
	const bool padding = size % 2 != (padEven ? 0 : 1);
	if(padding) // We will add padding, so make space for it
		size++;

	// Prepend the magic, 16-bit little-endian chip ID, and 16-bit little-endian size
	const uint16_t chipID = uint16_t(targetChip.getID());
	args.insert(args.begin(), {
		REDPacketizer::Magic,
		uint8_t(chipID & 0xFF), uint8_t(chipID >> 8),
		uint8_t(size & 0xFF), uint8_t(size >> 8)
	});

	if(padding)
		args.push_back(PaddingByte); // Ignored by the firmware

	// Write it to the device
	return device.com->sendPacket(args);
}

} // namespace Bootloader

} // namespace icsneo
