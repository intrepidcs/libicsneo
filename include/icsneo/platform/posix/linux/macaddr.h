#ifndef __MACADDR_LINUX_H_
#define __MACADDR_LINUX_H_

#include <netpacket/packet.h>

#define ICSNEO_AF_MACADDR (AF_PACKET)

// Points towards the first byte of the MAC address, given a sockaddr*
#define PLATFORM_MAC_FROM_SOCKADDR(SADDR) (((struct sockaddr_ll*)SADDR)->sll_addr)

#endif