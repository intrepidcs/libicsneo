#ifndef __BOOTLOADER_REDPACKETIZER_H_
#define __BOOTLOADER_REDPACKETIZER_H_

#ifdef __cplusplus

#include "icsneo/core/ringbuffer.h"

#include <cstdint>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * Splits bytes from a RED bootloader into frames:
 *   uint8_t  Magic
 *   uint8_t  Response
 *   uint16_t length (little-endian)
 *   uint8_t  data[length]
 */
class REDPacketizer {
public:
	static constexpr uint8_t Magic = 0xAA;

	enum class Response : uint8_t {
		Accepted = 0x77,
		Rejected = 0x7F,
	};

	struct RxFrame {
		uint16_t len = 0;
		Response resp = Response::Rejected;
		std::vector<uint8_t> data;
		bool done = false;
	};

	/**
	 * Call with bytes received from the device.
	 * The returned boolean indicates if there is output waiting in output()
	 */
	bool input(icsneo::RingBuffer& bytes);
	std::vector<RxFrame> output();
private:
	enum ReadState { HEADER, CMD, LENGTH_LOWER, LENGTH_UPPER, DATA };
	ReadState state = HEADER;
	std::vector<RxFrame> working;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_REDPACKETIZER_H_
