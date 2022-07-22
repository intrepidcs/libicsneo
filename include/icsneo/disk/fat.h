#ifndef __FAT_H__
#define __FAT_H__

#ifdef __cplusplus

#include <optional>
#include <cstdint>
#include <functional>

namespace icsneo {

namespace Disk {

std::optional<uint64_t> FindVSAInFAT(std::function< std::optional<uint64_t>(uint64_t pos, uint8_t* into, uint64_t amount) > diskRead);

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus

#endif // __FAT_H__