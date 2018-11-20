#ifndef __ICSNEO_API_VERSION_H_
#define __ICSNEO_API_VERSION_H_

#include <stdint.h>

#ifdef __cplusplus
#include <ostream>
#define NEOVERSION_DEFINE_BEGIN struct neoversion_t {
#else
#define NEOVERSION_DEFINE_BEGIN typedef struct {
#endif

NEOVERSION_DEFINE_BEGIN
	uint16_t major;
	uint16_t minor;
	uint16_t patch;
	const char* metadata;
	const char* buildBranch;
	const char* buildTag;
	char reserved[32];
#ifdef __cplusplus
	friend std::ostream& operator<<(std::ostream& os, const neoversion_t& version);
};
#else
} neoversion_t;
#endif

#ifdef __cplusplus
namespace icsneo {

neoversion_t GetVersion(void);

}
#endif

#endif