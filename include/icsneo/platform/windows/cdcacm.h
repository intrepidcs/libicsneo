#ifndef __CDCACM_WINDOWS_H_
#define __CDCACM_WINDOWS_H_

#ifdef __cplusplus

#include "icsneo/platform/windows/vcp.h"

namespace icsneo {

class CDCACM : public VCP {
public:
	CDCACM(const device_eventhandler_t& err, neodevice_t& forDevice) : VCP(err, forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product) { return VCP::FindByProduct(product, { L"usbser" }); }
};

}

#endif // __cplusplus

#endif