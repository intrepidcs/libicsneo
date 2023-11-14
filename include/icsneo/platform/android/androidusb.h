#ifndef __CDCACM_ANDROID_H_
#define __CDCACM_ANDROID_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/api/eventmanager.h"
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
	ANDROIDUSB(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err), device(forDevice) {}
	~ANDROIDUSB();
	static void Find(std::vector<FoundDevice>& found);

	bool open() override;
	bool isOpen() override;
	bool close() override;

	void modeChangeIncoming() override;
	void awaitModeChangeComplete() override;

    static void addSystemFD(int fd);
    static void removeSystemFD(int fd);

private:
	neodevice_t& device;
	static std::unordered_map<int,FoundDevice> systemFDs;
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

#endif //LIBICSNEO_TEST_ANDROIDUSB_H
