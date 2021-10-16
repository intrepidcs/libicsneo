#ifndef __UNALIGNED_H_
#define __UNALIGNED_H_

#if defined(_MSC_VER)
//#define ICSNEO_UNALIGNED(x) __unaligned x
#define ICSNEO_UNALIGNED(x) x
#else
#define ICSNEO_UNALIGNED(x) x
#endif

#endif