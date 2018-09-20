#ifndef __DYNAMICLIB_H_
#define __DYNAMICLIB_H_

#if defined _WIN32
#include "platform/windows/include/dynamiclib.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "platform/posix/include/dynamiclib.h"
#else
#warning "This platform is not supported by the dynamic library driver"
#endif

#endif