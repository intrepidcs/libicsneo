#ifndef __FTDI_WINDOWS_H_
#define __FTDI_WINDOWS_H_

#include "platform/windows/include/vcp.h"

namespace icsneo {

class FTDI : public VCP {
public:
	FTDI(neodevice_t& forDevice) : VCP(forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product) { return VCP::FindByProduct(product, L"serenum"); }
};

};

#endif