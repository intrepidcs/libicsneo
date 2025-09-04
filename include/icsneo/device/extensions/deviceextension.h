#ifndef __DEVICEEXTENSION_H_
#define __DEVICEEXTENSION_H_

#ifdef __cplusplus

#include <memory>
#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/device/device.h"

namespace icsneo {

class DeviceExtension {
public:
	DeviceExtension(Device& device) : device(device) {}
	virtual ~DeviceExtension() = default;
	virtual const char* getName() const = 0;

	// Return false to block opening
	virtual bool onDeviceOpen(Device::OpenFlags, const Device::OpenStatusHandler&) { return true; }

	// Return true to indicate that communication should now be back
	virtual bool onDeviceCommunicationDead(Device::OpenFlags, const Device::OpenStatusHandler&) { return false; }

	virtual void onGoOnline() {}
	virtual void onGoOffline() {}
	virtual void onDeviceClose() {}

	virtual void setGenericData(void*) {} 
	virtual bool providesFirmware() const { return false; }

	virtual void handleMessage(const std::shared_ptr<Message>&) {}

	// Return true to continue transmitting, success should be written to if false is returned
	virtual bool transmitHook(const std::shared_ptr<Frame>& frame, bool& success) { (void)frame; (void)success; return true; }

protected:
	Device& device;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __DEVICEEXTENSION_H_