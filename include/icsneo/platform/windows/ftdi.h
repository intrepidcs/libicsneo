#ifndef __FTDI_WINDOWS_H_
#define __FTDI_WINDOWS_H_

#ifdef __cplusplus

#include "icsneo/platform/windows/vcp.h"

namespace icsneo {

class FTDI : public VCP {
public:
	FTDI(const device_eventhandler_t& err, neodevice_t& forDevice) : VCP(err, forDevice) {}
	static void Find(std::vector<FoundDevice>& found) { return VCP::Find(found, { L"serenum" /*, L"ftdibus" */ }); }
};

}

#endif // __cplusplus

#endif