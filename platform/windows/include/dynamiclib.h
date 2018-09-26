#ifndef __DYNAMICLIB_WINDOWS_H_
#define __DYNAMICLIB_WINDOWS_H_

#include <Windows.h>

#ifdef ICSNEOC_MAKEDLL
#define DLLExport __declspec(dllexport)
#else
#define DLLExport __declspec(dllimport)
#endif

// MSVC does not have the ability to specify a destructor
#define ICSNEO_DESTRUCTOR

#define icsneo_dynamicLibraryLoad() LoadLibrary(L"C:\\Users\\Phollinsky\\Code\\icsneonext\\build\\icsneoc.dll")
#define icsneo_dynamicLibraryGetFunction(handle, func) GetProcAddress((HMODULE) handle, func)
#define icsneo_dynamicLibraryClose(handle) FreeLibrary((HMODULE) handle)

#endif