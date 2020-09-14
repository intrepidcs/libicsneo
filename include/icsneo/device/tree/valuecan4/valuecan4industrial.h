#ifndef __VALUECAN4INDUSTRIAL_H_
#define __VALUECAN4INDUSTRIAL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4industrialsettings.h"

namespace icsneo {

class ValueCAN4Industrial : public ValueCAN4 {
public:
	// Serial numbers start with IV for Industrial
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN4_IND;
	static constexpr const char* SERIAL_START = "IV";

protected:
	ValueCAN4Industrial(neodevice_t neodevice) : ValueCAN4(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}
};

}

#endif // __cplusplus

#endif