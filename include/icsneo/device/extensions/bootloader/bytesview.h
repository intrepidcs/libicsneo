#ifndef __BOOTLOADER_BYTESVIEW_H_
#define __BOOTLOADER_BYTESVIEW_H_

#ifdef __cplusplus

#include <cstdint>
#include <cstddef>

namespace icsneo {

namespace Bootloader {

class BytesView {
private:
	const uint8_t* ptr = nullptr;
	size_t length = 0;
public:
	BytesView() = default;
	BytesView(const uint8_t* buf, size_t size) : ptr(buf), length(size) {}

	const uint8_t* begin() const {
		return ptr;
	}

	const uint8_t* end() const {
		return ptr + length;
	}

	const uint8_t* data() const {
		return ptr;
	}

	size_t size() const {
		return length;
	}

	const uint8_t& operator[](size_t index) const {
		return ptr[index];
	}
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_BYTESVIEW_H_
