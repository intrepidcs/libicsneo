#ifndef __FTDI_H_
#define __FTDI_H_

#define INTREPID_USB_VENDOR_ID (0x093c)

#if defined _WIN32
#include "icsneo/platform/windows/ftdi.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/ftdi.h"
#else
#warning "This platform is not supported by the FTDI driver"
#endif

#endif