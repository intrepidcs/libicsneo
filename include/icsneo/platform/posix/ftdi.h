#ifndef __FTDI_POSIX_H_
#define __FTDI_POSIX_H_

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <ftdi.hpp>
#include "icsneo/device/neodevice.h"
#include "icsneo/communication/icommunication.h"
#include "icsneo/third-party/concurrentqueue/blockingconcurrentqueue.h"

namespace icsneo {

class FTDI : public ICommunication {
public:
	static constexpr neodevice_handle_t INVALID_HANDLE = 0x7fffffff; // int32_t max value
	static std::vector<neodevice_t> FindByProduct(int product);
	static bool IsHandleValid(neodevice_handle_t handle);

	FTDI(neodevice_t& forDevice);
	~FTDI() { close(); }
	bool open();
	bool close();
	bool isOpen() { return ftdiDevice.is_open(); }

private:
	static Ftdi::Context context;
	static neodevice_handle_t handleCounter;
	class FTDIDevice : public Ftdi::Context {
	public:
		FTDIDevice() {}
		FTDIDevice(const Ftdi::Context &x) : Ftdi::Context(x) {
			handle = handleCounter++;
		}
		neodevice_handle_t handle = INVALID_HANDLE;
	};
	static std::vector<FTDIDevice> searchResultDevices;
	static bool GetDeviceForHandle(neodevice_handle_t handle, FTDIDevice& device);
	
	void readTask();
	void writeTask();
	bool openable; // Set to false in the constructor if the object has not been found in searchResultDevices

	neodevice_t& device;
	FTDIDevice ftdiDevice;
};

}

#endif