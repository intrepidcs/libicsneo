#ifndef __STM32_H_
#define __STM32_H_

#define INTREPID_USB_VENDOR_ID (0x093c)

#if defined _WIN32
#include "icsneo/platform/windows/stm32.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/stm32.h"
#else
#warning "This platform is not supported by the STM32 driver"
#endif

#endif