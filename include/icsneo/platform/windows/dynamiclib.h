#ifndef __DYNAMICLIB_WINDOWS_H_
#define __DYNAMICLIB_WINDOWS_H_

#include "icsneo/platform/windows.h"

#ifndef ICSNEOC_BUILD_STATIC
#ifdef ICSNEOC_MAKEDLL
#define DLLExport __declspec(dllexport)
#else
#define DLLExport __declspec(dllimport)
#endif
#else
#define DLLExport
#endif


#define LegacyDLLExport DLLExport _stdcall

// MSVC does not have the ability to specify a destructor
#define ICSNEO_DESTRUCTOR

#define icsneo_dynamicLibraryLoad() LoadLibrary(TEXT("icsneoc.dll"))
#define icsneo_dynamicLibraryGetFunction(handle, func) GetProcAddress((HMODULE) handle, func)
#define icsneo_dynamicLibraryClose(handle) FreeLibrary((HMODULE) handle)

#endif