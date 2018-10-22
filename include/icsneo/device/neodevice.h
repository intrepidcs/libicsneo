#ifndef __NEODEVICE_H_
#define __NEODEVICE_H_

#include <stdint.h>
#include "icsneo/device/devicetype.h"

#ifdef __cplusplus
// A forward declaration is needed as there is a circular dependency
namespace icsneo {

class Device;

}
typedef icsneo::Device* devicehandle_t;
#else
typedef void* devicehandle_t;
#endif

typedef int32_t neodevice_handle_t;

#pragma pack(push, 1)

typedef struct {
	devicehandle_t device; // Pointer back to the C++ device object
	neodevice_handle_t handle; // Handle for use by the underlying driver
	devicetype_t type;
	char serial[7];
} neodevice_t;

#pragma pack(pop)

#endif