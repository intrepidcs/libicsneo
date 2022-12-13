#ifndef __CDCACM_POSIX_H_
#define __CDCACM_POSIX_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/api/eventmanager.h"
#include <optional>
#include <chrono>
#include <sys/stat.h>
#include <stdint.h>

namespace icsneo {

class CDCACM : public Driver {
public:
	/**
	 * Note: This is a driver for all devices which use CDC_ACM
	 * Once we find the TTY we want it's a pretty generic POSIX TTY driver, but
	 * the method for finding the TTY we want varies by OS.
	 * On Linux, we read sysfs to find users of the CDC_ACM driver
	 * On macOS, we use IOKit to find the USB device we're looking for
	 * As such platform specific FindByProduct & HandleToTTY code can be found
	 * in cdcacmlinux.cpp and cdcacmdarwin.cpp respectively
	 * Other POSIX systems (BSDs, QNX, etc) will need bespoke code written in the future
	 */
	CDCACM(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err, forDevice) {}
	~CDCACM();
	static void Find(std::vector<FoundDevice>& found);

	bool open() override;
	bool isOpen() override;
	bool close() override;

	void modeChangeIncoming() override;
	void awaitModeChangeComplete() override;

private:
	int fd = -1;
	std::optional<ino_t> disallowedInode;
	std::atomic<bool> modeChanging{false};
	std::thread modeChangeThread;
	std::mutex modeChangeMutex;
	std::condition_variable modeChangeCV;

	static std::string HandleToTTY(neodevice_handle_t handle);

	void readTask() override;
	void writeTask() override;
	bool fdIsValid();
};

}

#endif // __cplusplus

#endif