#ifndef __ICSNEO_NODISCARD_H_
#define __ICSNEO_NODISCARD_H_

/**
 * Allow the use of the nodiscard attribute where it is supported.
 */
#if __cplusplus > 201703L // C++20 and above
#define NODISCARD(str) [[nodiscard(str)]]
#elif __cplusplus > 201402L // C++17 and above
#define NODISCARD(str) [[nodiscard]]
#else
#define NODISCARD(str)
#endif

#endif