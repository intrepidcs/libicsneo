#ifndef __MACADDR_POSIX_H_
#define __MACADDR_POSIX_H_

#ifdef __APPLE__
#include "icsneo/platform/posix/darwin/macaddr.h"
#else
#include "icsneo/platform/posix/linux/macaddr.h"
#endif

#endif