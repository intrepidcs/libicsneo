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
    int ret = 0;
    if (!isOpen()) {
        report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
        return false;
    }

    /*
    struct termios tty = {};
    struct termios compare = {};

    if(tcgetattr(fd, &tty) != 0) {
        close();
        report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
        report(APIEvent::Type::DriverTCGetAddrFail, APIEvent::Severity::Error);
        return false;
    }

    tty.c_cflag |= (CLOCAL | CREAD); // Ignore modem controls
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8-bit characters
    tty.c_cflag &= ~PARENB; // No parity bit
    tty.c_cflag &= ~CSTOPB; // One stop bit
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control

    // Non-canonical mode
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    // Fetch bytes as they become available
    // See http://man7.org/linux/man-pages/man3/termios.3.html
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1; // 100ms timeout (1 decisecond, what?)

    if(tcsetattr(fd, TCSAFLUSH, &tty) != 0) { // Flushes input and output buffers as well as setting settings
        close();
        report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
        report(APIEvent::Type::DriverTCSetAddrFail, APIEvent::Severity::Error);
        return false;
    }

    if(tcgetattr(fd, &compare) != 0 || memcmp(&tty, &compare, sizeof(struct termios)) != 0) {
        close();
        return false;
    }
     */

    // Create threads
    readThread = std::thread(&ANDROIDUSB::readTask, this);
    writeThread = std::thread(&ANDROIDUSB::writeTask, this);

    return true;
}

bool ANDROIDUSB::isOpen() {
    return device.handle >= 0; // Negative fd indicates error or not opened yet
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
    device.handle = -1;

    uint8_t flush;
    WriteOperation flushop;
    while (readQueue.try_dequeue(flush)) {}
    while (writeQueue.try_dequeue(flushop)) {}

    return true;
}

void ANDROIDUSB::readTask() {
    //constexpr size_t READ_BUFFER_SIZE = 2048;
    //uint8_t readbuf[READ_BUFFER_SIZE];
    EventManager::GetInstance().downgradeErrorsOnCurrentThread();
    while(!closing && !isDisconnected()) {
        //fd_set rfds = {0};
        //struct timeval tv = {0};
        //FD_SET(fd, &rfds);
        //tv.tv_usec = 50000; // 50ms
        //::select(fd + 1, &rfds, NULL, NULL, &tv);
        ssize_t bytesRead = 0; // ::read(fd, readbuf, READ_BUFFER_SIZE);

        //libusb bulk transfer?
        //todo!

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

            //readQueue.enqueue_bulk(readbuf, bytesRead);
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
    while(!closing && !isDisconnected()) {
        if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
            continue;

        const ssize_t totalWriteSize = (ssize_t)writeOp.bytes.size();
        ssize_t totalWritten = 0;
        while(totalWritten < totalWriteSize) {
            const ssize_t writeSize = totalWriteSize - totalWritten;
            ssize_t actualWritten = writeSize; //::write(fd, writeOp.bytes.data() + totalWritten, writeSize);

            //libusb bulk transfer?
            //todo!

            if(actualWritten != writeSize) {
                // If we partially wrote, it's probably EAGAIN but it won't have been set
                // so don't signal an error unless it's < 0, we'll come back around and
                // get a -1 to see the real error.
                if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    // We filled the TX FIFO, use select to wait for it to become available again
                    //fd_set wfds = {0};
                    //struct timeval tv = {0};
                    //FD_SET(fd, &wfds);
                    //tv.tv_usec = 50000; // 50ms
                    //::select(fd + 1, nullptr, &wfds, nullptr, &tv);
                } else if (actualWritten < 0) {
                    if(!fdIsValid()) {
                        if(!isDisconnected()) {
                            disconnected = true;
                            report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
                        }
                    } else
                        report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
                    break;
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
    //libusb validate FD
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
    LOGD("libusb_init completed: %d\n", ret);
    LOGD("SYSTEM FDs size: %d\n", systemFDs.size());
    for (auto & [fd, libUsbHandle]: systemFDs) {
        ret = libusb_wrap_sys_device(ctx, (intptr_t) fd, &libusbDeviceHandle);
        LOGD("Wrapping system FD: %d, return: %s\n", fd, libusb_strerror(ret));
        if (ret == 0) {
            FoundDevice device = {};
            struct libusb_device_descriptor desc;
            unsigned char outString[255] = {};
            if(libusbDeviceHandle) {
                ret = libusb_get_device_descriptor(libusb_get_device(libusbDeviceHandle), &desc);
                if (ret == 0) {
                    device.productId = desc.idProduct;
                    LOGD("ProductID: %x\n", device.productId);
                    ret = libusb_get_string_descriptor_ascii(libusbDeviceHandle, desc.iSerialNumber,
                                                             outString, sizeof(outString));

                    if (ret > 0) {
                        std::strncpy(device.serial, reinterpret_cast<char *>(outString),
                                     sizeof(device.serial));
                        LOGD("Serial Number: %s\n", device.serial);
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