#include "icsneo/icsneo.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/device/devicefinder.h"

#include <string>
#include <algorithm>

using namespace icsneo;

struct icsneoOpenOptions_t {
    /** Open the device in online mode on open. */
    bool goOnlineOnOpen = true;
    /** Enable message polling on open. */
    bool enablePollingOnOpen = true;
    /** Synchronize the device RTC on open. 
     * Useful for devices that don't have an RTC battery like ValueCAN4 */
    bool syncRTCOnOpen = true;
};

typedef struct icsneo_device_t {
    // This is the actual device handle, when its not open it will be NULL.
    std::shared_ptr<Device> device;

    icsneoOpenOptions_t openOptions;
} *icsneo_device_ptr_t;

icsneo_result_t icsneo_find(icsneo_device_ptr_t* devices, uint32_t* devices_count, void* reserved) {
    static std::deque<icsneo_device_t> found_icsneo_devices;
    if (!devices || !devices_count) {
        return icsneo_error_invalid_parameter;
    }
    
    // Find all devices and get the minimum size to process
    auto found_devices = DeviceFinder::FindAll();
    auto min_size = std::minmax(found_devices.size(), static_cast<size_t>(*devices_count)).first;
    // lets resize the array to the minimum size
    for (auto i = 0; i < min_size; i++) {
        const auto* dev = devices[i];
        dev = new icsneo_device_t {};
        dev->device = found_devices[i];
    }


    return icsneo_error_success;
}

icsneo_result_t icsneo_open(icsneo_device_ptr_t* device) {
    if (!device) {
        return icsneo_error_invalid_parameter;
    }


    return icsneo_error_success;
}
