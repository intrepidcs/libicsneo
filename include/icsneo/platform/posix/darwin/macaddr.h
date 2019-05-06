#ifndef __MACADDR_DARWIN_H_
#define __MACADDR_DARWIN_H_

#include <net/if_dl.h>

#define ICSNEO_AF_MACADDR (AF_LINK)

// Points towards the first byte of the MAC address, given a sockaddr*
#define PLATFORM_MAC_FROM_SOCKADDR(SADDR) (((struct sockaddr_dl*)SADDR)->sdl_data + ((struct sockaddr_dl*)SADDR)->sdl_nlen)

#endif