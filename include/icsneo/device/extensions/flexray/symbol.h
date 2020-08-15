#ifndef __FLEXRAYSYMBOL_H_
#define __FLEXRAYSYMBOL_H_

#ifdef __cplusplus

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

#endif // __cplusplus

#endif // __FLEXRAYSYMBOL_H_