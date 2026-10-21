#include "icsneo/device/extensions/bootloader/parser/fwzip.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <miniz.h>

namespace icsneo {

namespace Bootloader {

class ZipHandle {
public:
	ZipHandle() {
		mz_zip_zero_struct(&handle);
	}
	~ZipHandle() {
		mz_zip_reader_end(&handle);
	}

	operator mz_zip_archive*() {
		return &handle;
	}

private:
	mz_zip_archive handle;
};

// A zip firmware image holds a single entry, the raw binary to send to the device
FWZip::FWZip(std::vector<uint8_t>&& tagData)
	: handle(std::make_unique<ZipHandle>()), zipData(std::move(tagData)) {

	if(!mz_zip_reader_init_mem(*handle, zipData.data(), zipData.size(), 0)) {
		Log::Error(__func__, "The firmware zip could not be opened");
		return;
	}

	mz_zip_archive_file_stat file;
	if(!mz_zip_reader_file_stat(*handle, 0, &file)) {
		Log::Error(__func__, "The firmware zip is empty or its first entry could not be read");
	} else {
		uncompressedSize = static_cast<size_t>(file.m_uncomp_size);
	}
}

FWZip::~FWZip() = default;

bool FWZip::readAll(uint8_t* out) {
	if(!uncompressedSize) {
		return false;
	}

	if(!out) {
		Log::Error(__func__, "No output buffer for the firmware zip");
		return false;
	}

	if(!mz_zip_reader_extract_to_mem(*handle, 0, out, *uncompressedSize, 0)) {
		Log::Error(__func__, "The firmware zip could not be decompressed");
		return false;
	}
	return true;
}

bool FWZip::readAll(std::vector<uint8_t>& out) {
	if(!uncompressedSize) {
		return false;
	}
	out = std::vector<uint8_t>(*uncompressedSize, 0);
	return readAll(out.data());
}

} // namespace Bootloader

} // namespace icsneo
