#ifndef __UNALIGNED_H_
#define __UNALIGNED_H_

#if defined(MSVC)
#define ICSNEO_UNALIGNED(x) __unaligned x
#else
#define ICSNEO_UNALIGNED(x) x
#endif

#endif