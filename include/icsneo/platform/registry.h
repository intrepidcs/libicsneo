#ifndef __REGISTRY_H_
#define __REGISTRY_H_

#if defined _WIN32
#include "icsneo/platform/windows/registry.h"
#else
#warning "This platform is not supported by the registry driver"
#endif

#endif