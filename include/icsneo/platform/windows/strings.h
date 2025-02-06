#ifndef __WINDOWS_STRINGS_H__
#define __WINDOWS_STRINGS_H__

#ifdef __cplusplus


#include <string>

namespace icsneo {

// Helper function to convert UTF-16 to UTF-8 strings (wide to standard)
std::string convertWideString(const std::wstring& value);

// Helper function to convert UTF-8 to UTF-16 strings (standard to wide)
std::wstring convertStringToWide(const std::string& value);

};


#endif // __cplusplus

#endif // __WINDOWS_STRINGS_H__
