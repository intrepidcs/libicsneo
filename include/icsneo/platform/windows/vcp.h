#ifndef __VCP_WINDOWS_H_
#define __VCP_WINDOWS_H_

#ifdef __cplusplus

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "icsneo/device/neodevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

// Virtual COM Port Communication
class VCP : public Driver {
public:
	static void Find(std::vector<FoundDevice>& found, std::vector<std::wstring> driverName);
	static bool IsHandleValid(neodevice_handle_t handle);
	typedef void(*fn_boolCallback)(bool success);
	
	VCP(const device_eventhandler_t& err, neodevice_t& forDevice);
	virtual ~VCP();
	bool open() { return open(false); }
	void openAsync(fn_boolCallback callback);
	bool close();
	bool isOpen();
	
private:
	bool open(bool fromAsync);
	bool opening = false;
	neodevice_t& device;

	struct Detail;
	std::shared_ptr<Detail> detail;

	std::vector<std::shared_ptr<std::thread>> threads;
	std::thread readThread, writeThread;
	void readTask();
	void writeTask();
};

}

#endif // __cplusplus

#endif