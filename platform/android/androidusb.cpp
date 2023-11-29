#include "icsneo/platform/android/androidusb.h"
#include "icsneo/device/founddevice.h"
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include "libusb.h"
#include <android/log.h>
#define  LOG_TAG    "libicsneo_test_androidusb"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace icsneo;

ANDROIDUSB::~ANDROIDUSB() {
    if (isOpen())
        close();
}

bool ANDROIDUSB::addSystemFD(int sysFd) {
    if ((sysFd != -1) && (systemFDs[sysFd] == nullptr)) {
            return true;
    }
    return false;
}

bool ANDROIDUSB::removeSystemFD(int sysFd) {
    if (sysFd != -1) {
        return systemFDs.erase(sysFd);
    }
    return false;
}

bool ANDROIDUSB::open() {
    openStatus = true;
    if (!isOpen()) {
        report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
        LOGD("Failed isOpen check in open(): %ul\n", device.handle);
        return false;
    }
    // Create threads
    readThread = std::thread(&ANDROIDUSB::readTask, this);
    writeThread = std::thread(&ANDROIDUSB::writeTask, this);
    return true;
}

bool ANDROIDUSB::isOpen() {
    //LOGD("isOpen handle: %i, openStatus: %s\n", device.handle, openStatus?"true":"false");
    return ((device.handle >= 0) && (openStatus)); // Negative fd indicates error or not opened yet
}

bool ANDROIDUSB::close() {
    if(!isOpen() && !isDisconnected()) {
        report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
        return false;
    }

    closing = true;

    if(readThread.joinable())
        readThread.join();

    if(writeThread.joinable())
        writeThread.join();

    closing = false;
    disconnected = false;

    systemFDs[device.handle] = nullptr;
    openStatus = false;
    device.handle = -1;

    uint8_t flush;
    WriteOperation flushop;
    while (readQueue.try_dequeue(flush)) {}
    while (writeQueue.try_dequeue(flushop)) {}

    return true;
}

void ANDROIDUSB::readTask() {
    constexpr size_t READ_BUFFER_SIZE = 2048;
    uint8_t readbuf[READ_BUFFER_SIZE];
    EventManager::GetInstance().downgradeErrorsOnCurrentThread();
    libusb_device_handle *devh = nullptr;
    auto mapItr = systemFDs.find(device.handle);
    if (mapItr != systemFDs.end()) {
        devh = mapItr->second;
    }
    while(!closing && !isDisconnected()) {
        int bytesRead = 0;
        auto ret = libusb_bulk_transfer(devh, ep_in_addr, readbuf, READ_BUFFER_SIZE, &bytesRead, 50);
        if (ret == LIBUSB_ERROR_TIMEOUT) {
            continue;
        }
        if (ret == LIBUSB_ERROR_NO_DEVICE) {
            disconnected = true;
            report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
        }
        if(bytesRead > 0) {
#if 0 // Perhaps helpful for debugging :)
            std::cout << "Read data: (" << bytesRead << ')' << std::hex << std::endl;
			for(int i = 0; i < bytesRead; i += 16) {
				for(int j = 0; j < std::min<int>(bytesRead - i, 16); j++)
					std::cout << std::setw(2) << std::setfill('0') << uint32_t(readbuf[i+j]) << ' ';
				std::cout << std::endl;
			}
			std::cout << std::dec << std::endl;
#endif

            readQueue.enqueue_bulk(readbuf, bytesRead);
        } else {
            if(!closing && !fdIsValid() && !isDisconnected()) {
                disconnected = true;
                report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
            }
        }
    }
}

void ANDROIDUSB::writeTask() {
    WriteOperation writeOp;
    EventManager::GetInstance().downgradeErrorsOnCurrentThread();
    libusb_device_handle *devh = nullptr;
    auto mapItr = systemFDs.find(device.handle);
    if (mapItr != systemFDs.end()) {
        devh = mapItr->second;
    }
    while(!closing && !isDisconnected()) {
        if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
            continue;

        const ssize_t totalWriteSize = (ssize_t)writeOp.bytes.size();
        ssize_t totalWritten = 0;
        while(totalWritten < totalWriteSize) {
            const ssize_t writeSize = totalWriteSize - totalWritten;
            int actualWritten = 0;
            auto ret = libusb_bulk_transfer(devh, ep_out_addr, writeOp.bytes.data() + totalWritten, writeSize, &actualWritten, 100);
            if (ret < 0) {
                if (ret == LIBUSB_ERROR_NO_DEVICE) {
                    LOGD("Write task bulk transfer failed, device disconnected!\n");
                    if(!isDisconnected()) {
                        disconnected = true;
                        report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
                    }
                }
                if (ret != LIBUSB_ERROR_TIMEOUT) {
                    LOGD("Write task bulk transfer failed!\n%s", libusb_strerror(ret));
                    continue;
                }
            }
            if(actualWritten > 0) {
#if 0 // Perhaps helpful for debugging :)
                std::cout << "Wrote data: (" << actualWritten << ')' << std::hex << std::endl;
				for(int i = 0; i < actualWritten; i += 16) {
					for(int j = 0; j < std::min<int>(actualWritten - i, 16); j++)
						std::cout << std::setw(2) << std::setfill('0') << uint32_t(writeOp.bytes[totalWritten+i+j]) << ' ';
					std::cout << std::endl;
				}
				std::cout << std::dec << std::endl;
#endif

                totalWritten += actualWritten;
            }
        }
    }
}

bool ANDROIDUSB::fdIsValid() {
    if (device.handle != -1) {
        auto itr = systemFDs.find(device.handle);
        if ((itr != systemFDs.end()) && (itr->second != nullptr)) {
            return true;
        }
    }
    return false;
}

void ANDROIDUSB::Find(std::vector<FoundDevice>& found) {
    int ret = 0;
    libusb_context *ctx = NULL;
    libusb_device_handle *libusbDeviceHandle = NULL;
    ret = libusb_set_option(ctx, LIBUSB_OPTION_NO_DEVICE_DISCOVERY, NULL);
    if (ret != LIBUSB_SUCCESS) {
        LOGD("libusb_init failed: %d\n", ret);
        return;
    }
    ret = libusb_init(&ctx);
    if (ret < 0) {
        LOGD("libusb_init failed: %d\n", ret);
        return;
    }
    //LOGD("libusb_init completed: %d\n", ret);
    //LOGD("SYSTEM FDs size: %d\n", systemFDs.size());
    for (auto & [fd, libUsbHandle]: systemFDs) {
        ret = libusb_wrap_sys_device(ctx, (intptr_t) fd, &libusbDeviceHandle);
        //LOGD("Wrapping system FD: %d, return: %s\n", fd, libusb_strerror(ret));
        if (ret == 0) {
            FoundDevice device = {};
            struct libusb_device_descriptor desc;
            unsigned char outString[255] = {};
            if(libusbDeviceHandle) {
                ret = libusb_get_device_descriptor(libusb_get_device(libusbDeviceHandle), &desc);
                if (ret == 0) {
                    device.productId = desc.idProduct;
                    //LOGD("ProductID: %x\n", device.productId);
                    ret = libusb_get_string_descriptor_ascii(libusbDeviceHandle, desc.iSerialNumber,
                                                             outString, sizeof(outString));

                    if (ret > 0) {
                        std::strncpy(device.serial, reinterpret_cast<char *>(outString),
                                     sizeof(device.serial));
                    } else {
                        LOGD("Serial number fetch failed!\n");
                    }
                } else {
                    LOGD("Fetch device descriptor failed for FD: %d, %s\n", fd, libusb_strerror(ret));
                }
            } else {
                LOGD("LIBUSB Device handle was invalid!\n");
            }
            device.handle = fd;

            // Add a factory to make the driver
            device.makeDriver = [&](const device_eventhandler_t &report, neodevice_t &device) {
                return std::unique_ptr<Driver>(new ANDROIDUSB(report, device));
            };

            found.push_back(device); // Finally, add device to search results
            libUsbHandle = libusbDeviceHandle;
        }
    }
}