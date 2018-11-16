#ifndef __FTDI_WINDOWS_H_
#define __FTDI_WINDOWS_H_

#include "icsneo/platform/windows/vcp.h"

namespace icsneo {

class FTDI : public VCP {
public:
	FTDI(device_errorhandler_t err, neodevice_t& forDevice) : VCP(err, forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product) { return VCP::FindByProduct(product, { L"serenum", L"ftdibus" }); }
};

}

#endif