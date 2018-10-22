#ifndef __TCHAR_H_
#define __TCHAR_H_

#if defined _WIN32
// Windows does not need a TCHAR definition, as it is natively defined
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/tchar.h"
#else
#warning "Please add a definition for this platform's equivalent to TCHAR"
#endif

#endif