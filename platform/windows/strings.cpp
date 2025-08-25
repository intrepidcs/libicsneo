#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <icsneo/platform/windows/strings.h>

// Helper function to convert UTF-16 to UTF-8 strings (wide to standard)
std::string icsneo::convertWideString(const std::wstring& value) {
	// Get the width of the string (character count)
	int width = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)value.c_str(), -1, 0, 0, NULL, NULL);
	// Create the new string
	std::string new_string(width+1, '\0');
	// fill the new string with the converted characters
	WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)value.c_str(), -1, new_string.data(), width, NULL, NULL);
	return new_string;
};

// Helper function to convert UTF-8 to UTF-16 strings (standard to wide)
std::wstring icsneo::convertStringToWide(const std::string& value) {
    // Get the width of the string (character count)
	int width = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, NULL, 0);
    // Create the new string
	std::wstring new_string(width+1, '\0');
    // fill the new string with the converted characters
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, (LPWSTR)new_string.data(), width);
    return new_string;
}