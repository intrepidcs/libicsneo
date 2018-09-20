#ifndef __DEVICES_H_
#define __DEVICES_H_

#if defined _WIN32
#include "platform/windows/include/devices.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "platform/posix/include/devices.h"
#else
#error "This platform is not supported by the devices driver, please add a definition!"
#endif

#endif