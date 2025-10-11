#ifndef __CDCACM_WINDOWS_H_
#define __CDCACM_WINDOWS_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>

namespace icsneo {

class CDCACM : public Driver {
public:
	CDCACM(const device_eventhandler_t& err, const std::wstring& path);
	static void Find(std::vector<FoundDevice>& found);
	bool open() override;
	bool isOpen() override;
	bool close() override;
	driver_finder_t getFinder() override { return CDCACM::Find; }

private:
	void read();
	void write();
	std::wstring path;
	HANDLE handle = INVALID_HANDLE_VALUE;
	std::thread readThread;
	std::thread writeThread;
	OVERLAPPED readOverlapped = {};
	OVERLAPPED writeOverlapped = {};
};

}

#endif // __cplusplus

#endif