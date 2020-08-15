#ifndef __FLEXRAYCRCSTATUS_H_
#define __FLEXRAYCRCSTATUS_H_

#ifdef __cplusplus

#include <cstdint>

namespace icsneo {

namespace FlexRay {

enum class CRCStatus {
	OK = 0,
	Error,
	NoCRC
};

} // namespace FlexRay

} // namespace icsneo

#endif // __cplusplus

#endif // __FLEXRAYCRCSTATUS_H_