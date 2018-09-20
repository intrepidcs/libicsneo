#ifndef __REGISTRY_WINDOWS_H_
#define __REGISTRY_WINDOWS_H_

#include <Windows.h>
#include <string>

namespace icsneo {

class Registry {
public:
	// Get string value
	static bool Get(std::wstring path, std::wstring key, std::wstring& value);
	static bool Get(std::string path, std::string key, std::string& value);

	// Get DWORD value
	static bool Get(std::wstring path, std::wstring key, uint32_t& value);
	static bool Get(std::string path, std::string key, uint32_t& value);

private:
	class Key {
	public:
		Key(std::wstring path, bool readwrite = false);
		~Key();
		HKEY GetKey() { return key; }
		bool IsOpen() { return key != nullptr; }
	private:
		HKEY key;
	};
};

};

#endif