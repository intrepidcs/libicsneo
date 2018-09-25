#ifndef __STM32_WINDOWS_H_
#define __STM32_WINDOWS_H_

#include "platform/windows/include/vcp.h"

namespace icsneo {

class STM32 : public VCP {
public:
	STM32(neodevice_t& forDevice) : VCP(forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product) { return VCP::FindByProduct(product, L"usbser"); }
};

}

#endif