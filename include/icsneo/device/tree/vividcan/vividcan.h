#ifndef __VIVIDCAN_H_
#define __VIVIDCAN_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/cdcacm.h"
#include "icsneo/device/tree/vividcan/vividcansettings.h"

namespace icsneo {

class VividCAN : public Device {
public:
	// Serial numbers start with VV
	// USB PID is 0x1102, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(VividCAN, DeviceType::VividCAN, "VV");

	// VividCAN does not go online, you can only set settings
	bool goOnline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool goOffline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool isOnlineSupported() const override { return false; }

	bool supportsComponentVersions() const override { return true; }

protected:
	VividCAN(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<VividCANSettings>(makeDriver);
	}

	bool requiresVehiclePower() const override { return false; }
	
	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*128;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif // __cplusplus

#endif