#ifndef __VCP_WINDOWS_H_
#define __VCP_WINDOWS_H_

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <Windows.h>
#include "device/include/neodevice.h"
#include "communication/include/icommunication.h"

namespace icsneo {

// Virtual COM Port Communication
class VCP : public ICommunication {
public:
	static std::vector<neodevice_t> FindByProduct(int product, wchar_t* driverName);
	static bool IsHandleValid(neodevice_handle_t handle);
	typedef void(*fn_boolCallback)(bool success);
	
	VCP(neodevice_t& forDevice) : device(forDevice) {
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
	HANDLE handle = INVALID_HANDLE_VALUE;
	OVERLAPPED overlappedRead = {};
	OVERLAPPED overlappedWrite = {};
	OVERLAPPED overlappedWait = {};
	std::vector<std::shared_ptr<std::thread>> threads;
	void readTask();
	void writeTask();
};

};

#endif