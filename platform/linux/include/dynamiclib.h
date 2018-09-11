#ifndef __DYNAMICLIB_H_LINUX_
#define __DYNAMICLIB_H_LINUX_

#include <dlfcn.h>

// Nothing special is needed to export
#define DLLExport

// #ifndef ICSNEO_NO_AUTO_DESTRUCT
// #define ICSNEO_DESTRUCTOR __attribute__((destructor));
// #else
#define ICSNEO_DESTRUCTOR
// #endif

#define icsneoDynamicLibraryLoad() dlopen("/media/paulywog/Windows 10/Users/phollinsky/Code/icsneonext/build/libicsneoc.so", RTLD_LAZY)
#define icsneoDynamicLibraryGetFunction(handle, func) dlsym(handle, func)
#define icsneoDynamicLibraryClose(handle) (dlclose(handle) == 0)

#endif