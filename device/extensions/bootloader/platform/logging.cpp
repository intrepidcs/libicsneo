#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <cstdlib>
#include <iostream>

namespace icsneo {

namespace Bootloader {

namespace Log {

// Set ICS_BOOTLOADER_LOG_LEVEL to 1 (error) through 5 (trace) to enable logging to stderr
static int GetLogLevel() {
	static const int level = [] {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // getenv
#endif
		const char* env = std::getenv("ICS_BOOTLOADER_LOG_LEVEL");
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		return env ? std::atoi(env) : 0;
	}();
	return level;
}

static void Write(int level, const char* prefix, const std::string& subject, const std::string& msg) {
	if(GetLogLevel() >= level)
		std::cerr << prefix << subject << " - " << msg << std::endl;
}

void Trace(const std::string& subject, const std::string& msg) {
	Write(5, "[TRACE] ", subject, msg);
}

void Debug(const std::string& subject, const std::string& msg) {
	Write(4, "[DEBUG] ", subject, msg);
}

void Info(const std::string& subject, const std::string& msg) {
	Write(3, "[INFO] ", subject, msg);
}

void Warning(const std::string& subject, const std::string& msg) {
	Write(2, "[WARN] ", subject, msg);
}

void Error(const std::string& subject, const std::string& msg) {
	Write(1, "[ERROR] ", subject, msg);
}

} // namespace Log

} // namespace Bootloader

} // namespace icsneo
