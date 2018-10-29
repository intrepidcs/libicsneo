#ifndef __ICSNEO_API_VERSION_H_
#define __ICSNEO_API_VERSION_H_

#include <stdint.h>

typedef struct {
	uint16_t major;
	uint16_t minor;
	uint16_t patch;
	const char* metadata;
	const char* buildBranch;
	const char* buildTag;
	char reserved[32];
} neoversion_t;

#ifdef __cplusplus
namespace icsneo {

neoversion_t GetVersion(void);

}
#endif

#endif