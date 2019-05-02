#ifndef __STM32_WINDOWS_H_
#define __STM32_WINDOWS_H_

#include "icsneo/platform/windows/vcp.h"

namespace icsneo {

class STM32 : public VCP {
public:
	STM32(const device_errorhandler_t& err, neodevice_t& forDevice) : VCP(err, forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product) { return VCP::FindByProduct(product, { L"usbser" }); }
};

}

#endif