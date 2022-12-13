#ifndef __SHAREDSEMAPHORE_H_
#define __SHAREDSEMAPHORE_H_

#ifdef _WIN32
#include "icsneo/platform/windows/sharedsemaphore.h"
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include "icsneo/platform/posix/sharedsemaphore.h"
#else
#warning "Shared semaphores are not supported on this platform"
#endif

#endif