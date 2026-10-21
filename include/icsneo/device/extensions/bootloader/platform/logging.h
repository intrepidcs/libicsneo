#ifndef __BOOTLOADER_LOGGING_H_
#define __BOOTLOADER_LOGGING_H_

#ifdef __cplusplus

#include <string>

namespace icsneo {

namespace Bootloader {

namespace Log {

void Trace(const std::string& subject, const std::string& msg);
void Debug(const std::string& subject, const std::string& msg);
void Info(const std::string& subject, const std::string& msg);
void Warning(const std::string& subject, const std::string& msg);
void Error(const std::string& subject, const std::string& msg);

} // namespace Log

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_LOGGING_H_
