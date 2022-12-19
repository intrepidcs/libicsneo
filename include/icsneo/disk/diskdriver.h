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
	Access getAccess() const {
		if(vsaOffset)
			return Access::VSA;
		return getPossibleAccess();
	}
	virtual std::pair<uint32_t, uint32_t> getBlockSizeBounds() const = 0;

	void setVSAOffset(uint64_t offset) { vsaOffset = offset; }
	uint64_t getVSAOffset() const { return vsaOffset; }

protected:
	uint64_t vsaOffset = 0;

private:
	/**
	 * Report the possible access that this driver has
	 * 
	 * In some cases, such as a mismatched possible access between
	 * read and write drivers, this will be overridden in the driver
	 * layer.
	 */
	virtual Access getPossibleAccess() const = 0;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus

#endif // __DISKDRIVER_H__