#ifndef __DISKDRIVER_H__
#define __DISKDRIVER_H__

#ifdef __cplusplus

#include <cstdint>
#include <chrono>
#include <utility>

namespace icsneo {

namespace Disk {

constexpr const std::chrono::milliseconds DefaultTimeout{2000};
constexpr const size_t SectorSize = 512;
enum class Access {
	None,
	EntireCard,
	VSA
};

/**
 * Interface for drivers which work with block data on devices
 */
class Driver {
public:
	virtual ~Driver() = default;
	virtual Access getAccess() const = 0;
	virtual std::pair<uint32_t, uint32_t> getBlockSizeBounds() const = 0;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus

#endif // __DISKDRIVER_H__