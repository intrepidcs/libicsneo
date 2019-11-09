#ifndef __FLEXRAYCHANNEL_H_
#define __FLEXRAYCHANNEL_H_

#include <cstdint>

namespace icsneo {

namespace FlexRay {

// In some cases used as a bitfield
enum class Channel {
	None = 0, // Usually an invalid/unset value
	A = 1,
	B = 2,
	AB = 3
};

} // namespace FlexRay

} // namespace icsneo

#endif // __FLEXRAYCHANNEL_H_