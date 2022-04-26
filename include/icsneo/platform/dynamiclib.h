#ifndef __DYNAMICLIB_H_
#define __DYNAMICLIB_H_

#if defined _WIN32
#include "icsneo/platform/windows/dynamiclib.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/dynamiclib.h"
#else
#warning "This platform is not supported by the dynamic library driver"
#endif

#endif