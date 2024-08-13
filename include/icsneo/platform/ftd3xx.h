#ifndef __FTD3XX_H_
#define __FTD3XX_H_

#ifdef __cplusplus

#include <optional>

#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"

namespace icsneo {

class FTD3XX : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);
	FTD3XX(const device_eventhandler_t& err, neodevice_t& forDevice);
	~FTD3XX() override { if(isOpen()) close(); }
	bool open() override;
	bool isOpen() override;
	bool close() override;
	bool isEthernet() const override { return false; }
private:
	neodevice_t& device;
	std::optional<void*> handle;

	std::thread readThread, writeThread;
	void readTask();
	void writeTask();
};

}

#endif // __cplusplus

#endif
