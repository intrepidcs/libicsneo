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
};

}

#endif // __cplusplus

#endif