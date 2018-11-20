#ifndef __VCP_WINDOWS_H_
#define __VCP_WINDOWS_H_

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <Windows.h>
#include "icsneo/device/neodevice.h"
#include "icsneo/communication/icommunication.h"
#include "icsneo/api/errormanager.h"

namespace icsneo {

// Virtual COM Port Communication
class VCP : public ICommunication {
public:
	static std::vector<neodevice_t> FindByProduct(int product, std::vector<std::wstring> driverName);
	static bool IsHandleValid(neodevice_handle_t handle);
	typedef void(*fn_boolCallback)(bool success);
	
	VCP(device_errorhandler_t err, neodevice_t& forDevice) : device(forDevice), err(err) {
		overlappedRead.hEvent = INVALID_HANDLE_VALUE;
		overlappedWrite.hEvent = INVALID_HANDLE_VALUE;
		overlappedWait.hEvent = INVALID_HANDLE_VALUE;
	}
	~VCP() { close(); }
	bool open() { return open(false); }
	void openAsync(fn_boolCallback callback);
	bool close();
	bool isOpen() { return handle != INVALID_HANDLE_VALUE; }
	
private:
	bool open(bool fromAsync);
	bool opening = false;
	neodevice_t& device;
	device_errorhandler_t err;
	HANDLE handle = INVALID_HANDLE_VALUE;
	OVERLAPPED overlappedRead = {};
	OVERLAPPED overlappedWrite = {};
	OVERLAPPED overlappedWait = {};
	std::vector<std::shared_ptr<std::thread>> threads;
	void readTask();
	void writeTask();
};

}

#endif