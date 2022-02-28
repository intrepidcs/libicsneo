#include "icsneo/platform/windows/registry.h"
#include "icsneo/platform/windows/windows.h"
#include <codecvt>
#include <vector>
#include <locale>

using namespace icsneo;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

class Key {
public:
	Key(std::wstring path, bool readwrite = false);
	~Key();
	HKEY GetKey() { return key; }
	bool IsOpen() { return key != nullptr; }
private:
	HKEY key;
};

Key::Key(std::wstring path, bool readwrite) {
	DWORD dwDisposition;
	if(readwrite)
		RegCreateKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, nullptr, 0, KEY_QUERY_VALUE | KEY_WRITE, nullptr, &key, &dwDisposition);
	else
		RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &key);
}

Key::~Key() {
	if(IsOpen())
		RegCloseKey(key);
}

bool Registry::EnumerateSubkeys(std::wstring path, std::vector<std::wstring>& subkeys) {
	Key regKey(path);
	if(!regKey.IsOpen())
		return false;

	wchar_t className[MAX_PATH];
	memset(className, 0, sizeof(className));
	DWORD classNameLen = MAX_PATH;
	DWORD subKeyCount = 0;
	DWORD maxSubKeyLen, maxClassStringLen, valueCount, maxValueNameLen, maxValueDataLen, securityDescriptorLen;
	FILETIME lastWriteTime;
	auto ret = RegQueryInfoKeyW(
		regKey.GetKey(),
		className,
		&classNameLen,
		nullptr,
		&subKeyCount,
		&maxSubKeyLen,
		&maxClassStringLen,
		&valueCount,
		&maxValueNameLen,
		&maxValueDataLen,
		&securityDescriptorLen,
		&lastWriteTime);

	if(ret != ERROR_SUCCESS)
		return false;
	
	subkeys.clear();
	for(DWORD i = 0; i < subKeyCount; i++) {
		DWORD nameLen = MAX_PATH;
		wchar_t name[MAX_PATH];
		memset(name, 0, sizeof(name));
		ret = RegEnumKeyExW(regKey.GetKey(), i, name, &nameLen, nullptr, nullptr, nullptr, &lastWriteTime);
		if(ret == ERROR_SUCCESS)
			subkeys.push_back(name);
	}
	return true;
}

bool Registry::Get(std::wstring path, std::wstring key, std::wstring& value) {
	Key regKey(path);
	if(!regKey.IsOpen())
		return false;
	
	// Query for the type and size of the data
	DWORD type, size;
	auto ret = RegQueryValueExW(regKey.GetKey(), key.c_str(), nullptr, &type, (LPBYTE)nullptr, &size);
	if(ret != ERROR_SUCCESS)
		return false;
	
	// Query for the data itself
	std::vector<wchar_t> data(size / 2 + 1);
	DWORD bytesRead = size; // We want to read up to the size we got earlier
	ret = RegQueryValueExW(regKey.GetKey(), key.c_str(), nullptr, &type, (LPBYTE)data.data(), &bytesRead);
	if(ret != ERROR_SUCCESS)
		return false;

	value = data.data();
	return true;
}

bool Registry::Get(std::string path, std::string key, std::string& value) {
	std::wstring wvalue;
	bool ret = Get(converter.from_bytes(path), converter.from_bytes(key), wvalue);
	value = converter.to_bytes(wvalue);
	return ret;
}

bool Registry::Get(std::wstring path, std::wstring key, uint32_t& value) {
	Key regKey(path);
	if(!regKey.IsOpen())
		return false;
	
	// Query for the data
	DWORD type, size = sizeof(DWORD), kvalue;
	auto ret = RegQueryValueExW(regKey.GetKey(), key.c_str(), nullptr, &type, (LPBYTE)&kvalue, &size);
	if(ret != ERROR_SUCCESS || type != REG_DWORD)
		return false;
	
	value = kvalue;
	return true;
}

bool Registry::Get(std::string path, std::string key, uint32_t& value) {
	return Get(converter.from_bytes(path), converter.from_bytes(key), value);
}
