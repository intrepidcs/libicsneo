#ifndef __VERSION_REPORT_H_
#define __VERSION_REPORT_H_

#ifdef __cplusplus

#include "icsneo/device/chipid.h"

#include <vector>
#include <string>

namespace icsneo {

struct VersionReport {
	ChipID id;
	std::string name;
	uint8_t major;
	uint8_t minor;
	uint8_t maintenance;
	uint8_t build;
};

}

#endif // __cplusplus

#endif