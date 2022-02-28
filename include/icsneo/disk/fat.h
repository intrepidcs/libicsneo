#ifndef __FAT_H__
#define __FAT_H__

#ifdef __cplusplus

#include "icsneo/platform/optional.h"
#include <cstdint>
#include <functional>

namespace icsneo {

namespace Disk {

optional<uint64_t> FindVSAInFAT(std::function< optional<uint64_t>(uint64_t pos, uint8_t* into, uint64_t amount) > diskRead);

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus

#endif // __FAT_H__