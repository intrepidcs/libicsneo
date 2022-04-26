#ifndef __DYNAMICLIB_POSIX_H_
#define __DYNAMICLIB_POSIX_H_

#include <dlfcn.h>

#ifdef __APPLE__
#include "icsneo/platform/posix/darwin/dynamiclib.h"
#else
#include "icsneo/platform/posix/linux/dynamiclib.h"
#endif

#define DLLExport __attribute__((visibility("default")))
#define LegacyDLLExport DLLExport

// #ifndef ICSNEO_NO_AUTO_DESTRUCT
// #define ICSNEO_DESTRUCTOR __attribute__((destructor));
// #else
#define ICSNEO_DESTRUCTOR
// #endif

#define icsneo_dynamicLibraryGetFunction(handle, func) dlsym(handle, func)
#define icsneo_dynamicLibraryClose(handle) (dlclose(handle) == 0)

#endif