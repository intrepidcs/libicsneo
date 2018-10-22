#include "icsneo/platform/windows/registry.h"
#include <codecvt>
#include <vector>

using namespace icsneo;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

Registry::Key::Key(std::wstring path, bool readwrite) {
	DWORD dwDisposition;
	if(readwrite)
		RegCreateKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, nullptr, 0, KEY_QUERY_VALUE | KEY_WRITE, nullptr, &key, &dwDisposition);
	else
		RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &key);
}

Registry::Key::~Key() {
	if(IsOpen())
		RegCloseKey(key);
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
	DWORD type, size, kvalue;
	auto ret = RegQueryValueExW(regKey.GetKey(), key.c_str(), nullptr, &type, (LPBYTE)&kvalue, &size);
	if(ret != ERROR_SUCCESS || type != REG_DWORD)
		return false;
	
	value = kvalue;
	return true;
}

bool Registry::Get(std::string path, std::string key, uint32_t& value) {
	return Get(converter.from_bytes(path), converter.from_bytes(key), value);
}