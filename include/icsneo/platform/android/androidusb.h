#ifndef __ANDROIDUSB_H_
#define __ANDROIDUSB_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/api/eventmanager.h"
#include "libusb.h"
#include <optional>
#include <chrono>
#include <sys/stat.h>
#include <stdint.h>
#include <unordered_map>

namespace icsneo {

class ANDROIDUSB : public Driver {
public:
	/**
	 * Note: This is a driver for all devices which use Android CDC_ACM
	 */
	ANDROIDUSB(const device_eventhandler_t& err, neodevice_t& forDevice)
    : Driver(err), device(forDevice) {
        if(auto key = systemFDs.find(device.handle); key != systemFDs.end())
            libusbDeviceHandle = key->second;
    }
	~ANDROIDUSB();
	static void Find(std::vector<FoundDevice>& found);

	bool open() override;
	bool isOpen() override;
	bool close() override;

    static bool addSystemFD(int fd);
    static bool removeSystemFD(int fd);

private:
	neodevice_t& device;
	libusb_context *ctx = nullptr;
	libusb_device_handle *libusbDeviceHandle = nullptr;
	inline static std::unordered_map<int,libusb_device_handle*> systemFDs; //android FD, libusb handle
	static constexpr int ep_in_addr = 0x83;
	static constexpr int ep_out_addr = 0x02;
	bool openStatus = false;
	void readTask() override;
	void writeTask() override;
	bool fdIsValid();
};

}

#endif // __cplusplus

#endif //LIBICSNEO_TEST_ANDROIDUSB_H
