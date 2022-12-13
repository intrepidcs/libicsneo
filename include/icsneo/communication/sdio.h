#ifndef __SDIO_H_
#define __SDIO_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/communication/interprocessmailbox.h"

namespace icsneo {

class SDIO : public Driver {
public:
	static void Find(std::vector<FoundDevice>& found);

	SDIO(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err, forDevice) {}
	~SDIO() { if(isOpen()) close(); }
	bool open() override;
	bool close() override;
	bool isOpen() override;
	bool enableHeartbeat() const override { return true; }

private:
	void readTask() override;
	void writeTask() override;
	bool deviceOpen = false;
	InterprocessMailbox outboundIO;
	InterprocessMailbox inboundIO;
};

}

#endif // __cplusplus

#endif
