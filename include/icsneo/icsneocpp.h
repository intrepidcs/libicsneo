#ifndef __ICSNEOCPP_H_
#define __ICSNEOCPP_H_

#include <vector>
#include <memory>

#include "icsneo/device/device.h"
#include "icsneo/api/version.h"
#include "icsneo/api/errormanager.h"

namespace icsneo {

std::vector<std::shared_ptr<Device>> FindAllDevices();
std::vector<DeviceType> GetSupportedDevices();

size_t ErrorCount(ErrorFilter filter = ErrorFilter());
std::vector<APIError> GetErrors(ErrorFilter filter, size_t max = 0);
std::vector<APIError> GetErrors(size_t max = 0, ErrorFilter filter = ErrorFilter());
void GetErrors(std::vector<APIError>& errors, ErrorFilter filter, size_t max = 0);
void GetErrors(std::vector<APIError>& errors, size_t max = 0, ErrorFilter filter = ErrorFilter());
bool GetLastError(APIError& error, ErrorFilter filter = ErrorFilter());
void DiscardErrors(ErrorFilter filter = ErrorFilter());
void SetErrorLimit(size_t newLimit);
size_t GetErrorLimit();

}

#endif