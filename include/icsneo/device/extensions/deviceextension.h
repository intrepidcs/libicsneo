#ifndef __DEVICEEXTENSION_H_
#define __DEVICEEXTENSION_H_

#include <memory>
#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

class Device;

class DeviceExtension {
public:
	DeviceExtension(Device& device) : device(device) {}
	virtual ~DeviceExtension() = default;
	virtual const char* getName() const = 0;
	virtual void handleMessage(const std::shared_ptr<Message>& message) {}

protected:
	Device& device;
};

} // namespace icsneo


#endif // __DEVICEEXTENSION_H_