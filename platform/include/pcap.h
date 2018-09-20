#ifndef __PCAP_H_
#define __PCAP_H_

#if defined _WIN32
#include "platform/windows/include/pcap.h"
// #elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
// #include "platform/posix/include/ftdi.h"
#else
#warning "This platform is not supported by the PCAP driver"
#endif

#endif