#ifndef __PCAP_H_
#define __PCAP_H_

#if defined _WIN32
#include "platform/windows/include/pcap.h"
// #elif defined __linux__
// #include "platform/linux/include/ftdi.h"
#else
#warning "This platform is not supported by the PCAP driver"
#endif

#endif