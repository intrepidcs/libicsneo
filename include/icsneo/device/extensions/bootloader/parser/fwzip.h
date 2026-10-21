#ifndef __BOOTLOADER_FWZIP_H_
#define __BOOTLOADER_FWZIP_H_

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace icsneo {

namespace Bootloader {

class ZipHandle;

class FWZip {
private:
	std::unique_ptr<ZipHandle> handle;
	std::vector<uint8_t> zipData;
	std::optional<size_t> uncompressedSize = std::nullopt;
public:
	FWZip(std::vector<uint8_t>&& tagData);
	~FWZip();

	std::optional<size_t> size() const { return uncompressedSize; }
	bool readAll(uint8_t* out);
	bool readAll(std::vector<uint8_t>& out);
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_FWZIP_H_
