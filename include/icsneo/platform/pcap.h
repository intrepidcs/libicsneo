#ifndef __PCAP_H_
#define __PCAP_H_

#if defined _WIN32
#include "icsneo/platform/windows/pcap.h"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "icsneo/platform/posix/pcap.h"
#else
#warning "This platform is not supported by the PCAP driver"
#endif

#define LIBICSNEO_HAVE_PCAP 1

#endif