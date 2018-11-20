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

std::ostream& operator<<(std::ostream& os, const neoversion_t& version) {
	os << 'v' << version.major << '.' << version.minor << '.' << version.patch;
	if(version.metadata[0] != '\0')
		os << '+' << version.metadata;
	if(std::string(version.buildBranch) != "master")
		os << ' ' << version.buildBranch << " @";
	if(version.buildTag[0] != 'v')
		os << ' ' << version.buildTag;
	return os;
}