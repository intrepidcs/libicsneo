#ifndef __FLEXRAYSYMBOL_H_
#define __FLEXRAYSYMBOL_H_

#include <cstdint>

namespace icsneo {

namespace FlexRay {

enum class Symbol {
	None = 0,
	Unknown,
	Wakeup,
	CAS
};

} // namespace FlexRay

} // namespace icsneo

#endif // __FLEXRAYSYMBOL_H_