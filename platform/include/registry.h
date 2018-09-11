#ifndef __REGISTRY_H_
#define __REGISTRY_H_

#if defined _WIN32
#include "platform/windows/include/registry.h"
#else
#warning "This platform is not supported by the registry driver"
#endif

#endif