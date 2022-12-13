#ifndef __SHAREDMEMORY_H_
#define __SHAREDMEMORY_H_

#ifdef _WIN32
#include "icsneo/platform/windows/sharedmemory.h"
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include "icsneo/platform/posix/sharedmemory.h"
#else
#warning "Shared memory are not supported on this platform"
#endif

#endif