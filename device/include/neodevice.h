#ifndef __NEODEVICE_H_
#define __NEODEVICE_H_

#include <stdint.h>

#ifdef __cplusplus
// A forward declaration is needed as there is a circular dependency
namespace icsneo {
	class Device;
};
typedef icsneo::Device* devicehandle_t;
#else
typedef void* devicehandle_t;
#endif

typedef int32_t neodevice_handle_t;

typedef struct {
	devicehandle_t device;
	neodevice_handle_t handle;
	char serial[7];
	char type[64];
} neodevice_t;

#endif