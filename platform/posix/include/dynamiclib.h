#ifndef __DYNAMICLIB_POSIX_H_
#define __DYNAMICLIB_POSIX_H_

#include <dlfcn.h>

#ifdef __APPLE__
#include "platform/posix/darwin/include/dynamiclib.h"
#else
#include "platform/posix/linux/include/dynamiclib.h"
#endif

// Nothing special is needed to export
#define DLLExport

// #ifndef ICSNEO_NO_AUTO_DESTRUCT
// #define ICSNEO_DESTRUCTOR __attribute__((destructor));
// #else
#define ICSNEO_DESTRUCTOR
// #endif

#define icsneo_dynamicLibraryGetFunction(handle, func) dlsym(handle, func)
#define icsneo_dynamicLibraryClose(handle) (dlclose(handle) == 0)

#endif