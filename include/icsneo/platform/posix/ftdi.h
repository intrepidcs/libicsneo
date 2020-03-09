#ifndef __FTDI_POSIX_H_
#define __FTDI_POSIX_H_

#include <vector>
#include <memory>
#include <string>
#include <ftdi.h>
#include "icsneo/device/neodevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/third-party/concurrentqueue/blockingconcurrentqueue.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

class FTDI : public Driver {
public:
	static std::vector<neodevice_t> FindByProduct(int product);

	FTDI(const device_eventhandler_t& err, neodevice_t& forDevice);
	~FTDI() { close(); }
	bool open();
	bool close();
	bool isOpen() { return ftdi.isOpen(); }

private:
	class FTDIContext {
	public:
		FTDIContext() : context(ftdi_new()) {}
		~FTDIContext() {
			if(context)
				ftdi_free(context); // calls ftdi_deinit and ftdi_close if required
			context = nullptr;
		}

		std::pair<int, std::vector<std::string>> findDevices(int pid);
		int openDevice(int pid, const char* serial);
		bool closeDevice();
		bool isOpen() const { return deviceOpen; }
		int flush() { return ftdi_usb_purge_buffers(context); }
		int reset() { return ftdi_usb_reset(context); }
		int read(uint8_t* data, size_t size) { return ftdi_read_data(context, data, (int)size); }
		int write(const uint8_t* data, size_t size) { return ftdi_write_data(context, data, (int)size); }
		int setBaudrate(int baudrate) { return ftdi_set_baudrate(context, baudrate); }
		int setLatencyTimer(uint8_t latency) { return ftdi_set_latency_timer(context, latency); }
		bool setReadTimeout(int timeout) { if(context == nullptr) return false; context->usb_read_timeout = timeout; return true; }
		bool setWriteTimeout(int timeout) { if(context == nullptr) return false; context->usb_write_timeout = timeout; return true; }
	private:
		struct ftdi_context* context;
		bool deviceOpen = false;
	};
	FTDIContext ftdi;

	static std::vector<std::tuple<int, std::string>> handles;

	void readTask();
	void writeTask();
	bool openable; // Set to false in the constructor if the object has not been found in searchResultDevices

	neodevice_t& device;
};

}

#endif