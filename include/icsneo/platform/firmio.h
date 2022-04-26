#ifndef __FIRMIO_H_
#define __FIRMIO_H_

#if defined (__linux__)
#include "icsneo/platform/posix/firmio.h"
#else
// This driver is only relevant for communication communication between
// Linux and CoreMini from the onboard processor of the device, you
// likely do not want it enabled for your build.
#warning "This platform is not supported by the firmio driver"
#endif

#endif