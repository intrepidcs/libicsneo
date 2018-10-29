#include "icsneo/api/version.h"
#include "generated/buildinfo.h"

using namespace icsneo;

neoversion_t icsneo::GetVersion(void) {
	neoversion_t version = {};
	version.major = BUILD_MAJOR;
	version.minor = BUILD_MINOR;
	version.patch = BUILD_PATCH;
	version.metadata = BUILD_METADATA;
	version.buildBranch = GIT_BRANCH;
	version.buildTag = GIT_DESCRIBE;
	return version;
}