// Include Windows.h with as few annoying defines as possible

#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define LAM_DEFINED
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#ifdef LAM_DEFINED
#undef LAM_DEFINED
#undef WIN32_LEAN_AND_MEAN
#endif
#undef NOMINMAX