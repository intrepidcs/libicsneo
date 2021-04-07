#ifndef __VALUECAN4_2EL_H_
#define __VALUECAN4_2EL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-2elsettings.h"
#include <string>

namespace icsneo {

class ValueCAN4_2EL : public ValueCAN4 {
public:
	// Serial numbers start with VE for 4-2EL
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_2EL;
	static constexpr const char* SERIAL_START = "VE";

protected:
	ValueCAN4_2EL(neodevice_t neodevice) : ValueCAN4(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	void handleDeviceStatus(const std::shared_ptr<Message>& message) override {
		if(!message || message->data.size() < sizeof(valuecan4_2el_status_t))
			return;
		const valuecan4_2el_status_t* status = reinterpret_cast<const valuecan4_2el_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}
};

}

#endif // __cplusplus

#endif