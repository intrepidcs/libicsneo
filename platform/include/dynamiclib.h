#ifndef __DYNAMICLIB_H_
#define __DYNAMICLIB_H_

#if defined _WIN32
#include "platform/windows/include/dynamiclib.h"
#elif defined __linux__
#include "platform/linux/include/dynamiclib.h"
#else
#warning "This platform is not supported by the dynamic library driver"
#endif

#endif