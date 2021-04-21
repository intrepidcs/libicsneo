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

	enum class SKU {
		Standard,
		AP04E0A_D26, // HDB26, USB A, and Keysight Branding
		AP04E0A_MUL, // Multi-connectors, USB A, and Keysight Branding
		AP04E0A_OBD, // OBD, USB A, and Keysight Branding
	};

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::AP04E0A_D26;
			case 'B':
				return SKU::AP04E0A_MUL;
			case 'C':
				return SKU::AP04E0A_OBD;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP04E0A_D26:
				return "Keysight AP04E0A-D26";
			case SKU::AP04E0A_MUL:
				return "Keysight AP04E0A-MUL";
			case SKU::AP04E0A_OBD:
				return "Keysight AP04E0A-OBD";
		}
		return Device::getProductName();
	}

protected:
	ValueCAN4_2EL(neodevice_t neodevice) : ValueCAN4(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	void handleDeviceStatus(const std::shared_ptr<Message>& message) override {
		if(!message || message->data.size() < sizeof(valuecan4_2el_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const valuecan4_2el_status_t* status = reinterpret_cast<const valuecan4_2el_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}
};

}

#endif // __cplusplus

#endif