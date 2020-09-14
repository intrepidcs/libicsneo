#ifndef __FTDI3_H_
#define __FTDI3_H_

#define INTREPID_USB_VENDOR_ID (0x093c)

// This is currently a stub for the FTDI3 driver,
// it uses the FTDI driver to find devices but will
// not allow them to connect!
#define FTDI3 FTDI

#if defined _WIN32
#include "icsneo/platform/windows/ftdi.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/ftdi.h"
#else
#warning "This platform is not supported by the FTDI driver"
#endif

#endif