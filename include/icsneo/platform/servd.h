#ifndef __SERVD_H_
#define __SERVD_H_

#ifdef __cplusplus

#include <optional>
#include <thread>
#include <unordered_set>

#include "icsneo/device/neodevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/platform/socket.h"

namespace icsneo {

class Servd : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);
	static bool Enabled();
	Servd(const device_eventhandler_t& err, neodevice_t& forDevice, const std::unordered_set<std::string>& availableDrivers);
	~Servd() override;
	bool open() override;
	bool isOpen() override;
	bool close() override;
	bool faa(const std::string& key, int32_t inc, int32_t& orig);
	bool enableCommunication(bool enable, bool& sendMsg) override;
	driver_finder_t getFinder() override { return Servd::Find; }

private:
	void alive();
	void read(Address&& address);
	void write(Address&& address);
	neodevice_t& device;
	std::thread aliveThread; // makes sure the client and server are healthy
	std::thread writeThread;
	std::thread readThread;
	Socket messageSocket;
	bool opened = false;
	bool comEnabled = false;
	std::string driver;
};

}

#endif // __cplusplus

#endif
