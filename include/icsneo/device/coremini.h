#ifndef __COREMINI_H_
#define __COREMINI_H_

#ifdef __cplusplus

#include <cstdint>
#include <array>
#include <chrono>

namespace icsneo {

struct CoreminiHeader {
	uint16_t coreminiVersion;
	uint32_t storedFileSize;
	// 32-bit word checksum on the entire (decompressed) binary, with the checksum and hash fields set to 0
	uint32_t fileChecksum;
	// SHA256 hash of the entire (decompressed) binary, with the checksum, hash, and create time fields set to 0
	bool skipDecompression;
	bool encryptedMode;
	std::array<uint8_t, 32> fileHash;
	std::chrono::time_point<std::chrono::system_clock> timestamp;
};

}

#endif // __cplusplus

#endif