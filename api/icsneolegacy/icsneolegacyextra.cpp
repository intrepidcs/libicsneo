// Legacy functions which access the C++ objects directly
// idevicesettings.h (from Device) and icsnVC40.h cannot be included together, hence this file

#ifndef __cplusplus
#error "icsneolegacyextra.cpp must be compiled with a C++ compiler!"
#endif

#include "icsneo/device/device.h"
#define ICSNEOC_MAKEDLL
#include "icsneo/platform/dynamiclib.h" // Dynamic library loading and exporting
#undef ICSNEOC_MAKEDLL
#include <chrono>

using namespace icsneo;

extern "C" {
extern int DLLExport icsneoValidateHObject(void* hObject);
extern int DLLExport icsneoWaitForRxMessagesWithTimeOut(void* hObject, unsigned int iTimeOut);
}

int icsneoWaitForRxMessagesWithTimeOut(void* hObject, unsigned int iTimeOut) {
	if(!icsneoValidateHObject(hObject))
		return false;
	neodevice_t* device = (neodevice_t*)hObject;
	if(device->device->getCurrentMessageCount() != 0)
		return true;
	return bool(device->device->com->waitForMessageSync(MessageFilter(), std::chrono::milliseconds(iTimeOut)));
}