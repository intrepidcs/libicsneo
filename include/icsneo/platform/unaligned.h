#ifndef __UNALIGNED_H_
#define __UNALIGNED_H_

#if defined(_MSC_VER) && !defined(_M_IX86)
#define ICSNEO_UNALIGNED(x) __unaligned x
#else
#define ICSNEO_UNALIGNED(x) x
#endif

#endif