#ifndef __CDCACM_H_
#define __CDCACM_H_

#define INTREPID_USB_VENDOR_ID (0x093c)

#if defined _WIN32
#include "icsneo/platform/windows/cdcacm.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/cdcacm.h"
#else
#warning "This platform is not supported by the CDC ACM driver"
#endif

#endif