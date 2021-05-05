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
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VividCAN;
	static constexpr const uint16_t PRODUCT_ID = 0x1102;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(PRODUCT_ID))
			found.emplace_back(new VividCAN(neodevice));

		return found;
	}

	// VividCAN does not go online, you can only set settings
	bool goOnline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool goOffline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

protected:
	bool requiresVehiclePower() const override { return false; }

private:
	VividCAN(neodevice_t neodevice) : Device(neodevice) {
		initialize<CDCACM, VividCANSettings>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif