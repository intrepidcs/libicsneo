#include "icsneo/device/extensions/bootloader/communication/communication.h"

using namespace icsneo::Bootloader;

uint32_t Communication::readLE32(const uint8_t* buf) {
	return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

void Communication::writeLE32(uint8_t* buf, uint32_t num) {
	buf[0] = num & 0xFFu;
	buf[1] = (num >> 8) & 0xFFu;
	buf[2] = (num >> 16) & 0xFFu;
	buf[3] = (num >> 24) & 0xFFu;
}
