#ifndef __DXX_H_
#define __DXX_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"

#include "libredxx/libredxx.h"

namespace icsneo {

class DXX : public Driver {
public:
	static void Find(std::vector<FoundDevice>& found);
	
	DXX(const device_eventhandler_t& err, neodevice_t& forDevice, uint16_t pid, libredxx_device_type type);

	bool open() override;

	bool isOpen() override;

	bool close() override;
	driver_finder_t getFinder() override { return DXX::Find; }

private:
	void read();
	void write();
	neodevice_t neodevice;
	uint16_t pid;
	libredxx_device_type type;
	libredxx_opened_device* device = nullptr;
	std::thread readThread;
	std::thread writeThread;
};

}

#endif // __cplusplus
#endif // __DXX_H_
