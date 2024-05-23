#include "icsneo/platform/ftdi.h"
#include "icsneo/device/founddevice.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <memory>
#include <utility>
#include <cctype>
#include <algorithm>
#include <libusb.h>

using namespace icsneo;

std::vector<std::string> FTDI::handles;

void FTDI::Find(std::vector<FoundDevice>& found) {
	constexpr size_t deviceSerialBufferLength = sizeof(device.serial);
	static FTDIContext context;

	const auto result = context.findDevices();
	if(result.first < 0)
		return; // TODO Flag an error for the client application, there was an issue with FTDI

	for(const auto& [serial, pid] : result.second) {
		FoundDevice d;
		strncpy(d.serial, serial.c_str(), deviceSerialBufferLength - 1);
		d.serial[deviceSerialBufferLength - 1] = '\0'; // strncpy does not write a null terminator if serial is too long
		for(size_t i = 0; i < deviceSerialBufferLength - 1; i++)
			d.serial[i] = toupper(serial[i]);
		std::string devHandle = serial;
		auto it = std::find(handles.begin(), handles.end(), devHandle);
		size_t foundHandle = SIZE_MAX;
		if(it != handles.end()) {
			foundHandle = it - handles.begin();
		} else {
			foundHandle = handles.size();
			handles.push_back(devHandle);
		}
		d.handle = foundHandle;
		d.productId = pid;

		d.makeDriver = [](const device_eventhandler_t& report, neodevice_t& device) {
			return std::unique_ptr<Driver>(new FTDI(report, device));
		};

		found.push_back(d);
	}
}

FTDI::FTDI(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err), device(forDevice) {
	openable = strlen(forDevice.serial) > 0 && device.handle >= 0 && device.handle < (neodevice_handle_t)handles.size();
}

bool FTDI::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}
		
	if(!openable) {
		report(APIEvent::Type::InvalidNeoDevice, APIEvent::Severity::Error);
		return false;
	}

	// At this point the handle has been checked to be within the bounds of the handles array
	auto& handle = handles[device.handle];
	const int openError = ftdi.openDevice(0, handle.c_str());
	if(openError == -5) { // Unable to claim device
		report(APIEvent::Type::DeviceInUse, APIEvent::Severity::Error);
		return false;
	} else if(openError != 0) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	ftdi.setReadTimeout(100);
	ftdi.setWriteTimeout(1000);
	ftdi.reset();
	ftdi.setBaudrate(500000);
	ftdi.setLatencyTimer(1);
	ftdi.flush();

	// Create threads
	closing = false;
	readThread = std::thread(&FTDI::readTask, this);
	writeThread = std::thread(&FTDI::writeTask, this);

	return true;
}

bool FTDI::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}
		
	closing = true;
	
	if(readThread.joinable())
		readThread.join();
	
	if(writeThread.joinable())
		writeThread.join();

	bool ret = true;
	if(!isDisconnected()) {
		ret = ftdi.closeDevice();
		if(!ret)
			report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
	}
	
	WriteOperation flushop;
	readBuffer.clear();
	while(writeQueue.try_dequeue(flushop)) {}

	closing = false;
	disconnected = false;
	return ret;
}

std::pair<int, std::vector< std::pair<std::string, uint16_t> > > FTDI::FTDIContext::findDevices(int pid) {
	std::pair<int, std::vector< std::pair<std::string, uint16_t> > > ret;
	
	if(context == nullptr) {
		ret.first = -1;
		return ret;
	}
	
	struct ftdi_device_list* devlist = nullptr;
	ret.first = ftdi_usb_find_all(context, &devlist, INTREPID_USB_VENDOR_ID, pid);
	if(ret.first < 1) {
		// Didn't find anything, maybe got an error
		if(devlist != nullptr)
			ftdi_list_free(&devlist);
		return ret;
	}
	
	if(devlist == nullptr) {
		ret.first = -4;
		return ret;
	}

	for(struct ftdi_device_list* curdev = devlist; curdev != nullptr; curdev = curdev->next) {
		struct libusb_device_descriptor descriptor = {};
		// Check against bDeviceClass here as it will be 0 for FTDI devices
		// It will be 2 for CDC ACM devices, which we don't want to handle here
		if(libusb_get_device_descriptor(curdev->dev, &descriptor) != 0 || descriptor.bDeviceClass != 0)
			continue;

		char serial[16] = {};
		if(ftdi_usb_get_strings(context, curdev->dev, nullptr, 0, nullptr, 0, serial, sizeof(serial)) < 0)
			continue;

		const auto len = strnlen(serial, sizeof(serial));
		if(len > 4 && len < 10)
			ret.second.emplace_back(serial, descriptor.idProduct);
	}

	ret.first = static_cast<int>(ret.second.size());
	ftdi_list_free(&devlist);
	return ret;
}

int FTDI::FTDIContext::openDevice(int pid, const char* serial) {
	if(context == nullptr)
		return 1;
	if(serial == nullptr)
		return 2;
	if(serial[0] == '\0')
		return 3;
	if(deviceOpen)
		return 4;
	int ret = ftdi_usb_open_desc(context, INTREPID_USB_VENDOR_ID, pid, nullptr, serial);
	if(ret == 0 /* all ok */)
		deviceOpen = true;
	return ret;
}

bool FTDI::FTDIContext::closeDevice() {
	if(context == nullptr)
		return false;
	
		
	if(!deviceOpen)
		return true;

	int ret = ftdi_usb_close(context);
	if(ret != 0)
		return false;
		
	deviceOpen = false;
	return true;
}

bool FTDI::ErrorIsDisconnection(int errorCode) {
	return errorCode == LIBUSB_ERROR_NO_DEVICE ||
		errorCode == LIBUSB_ERROR_PIPE ||
		errorCode == LIBUSB_ERROR_IO;
}

void FTDI::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 8;
	uint8_t readbuf[READ_BUFFER_SIZE];
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing && !isDisconnected()) {
		auto readBytes = ftdi.read(readbuf, READ_BUFFER_SIZE);
		if(readBytes < 0) {
			if(ErrorIsDisconnection(readBytes)) {
				if(!isDisconnected()) {
					disconnected = true;
					report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
				}
			} else
				report(APIEvent::Type::FailedToRead, APIEvent::Severity::EventWarning);
		} else
			writeToReadBuffer(readbuf, readBytes);
	}
}

void FTDI::writeTask() {
	WriteOperation writeOp;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing && !isDisconnected()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		size_t offset = 0;
		while(offset < writeOp.bytes.size()) {
			auto writeBytes = ftdi.write(writeOp.bytes.data() + offset, (int)writeOp.bytes.size() - offset);
			if(writeBytes < 0) {
				if(ErrorIsDisconnection(writeBytes)) {
					if(!isDisconnected()) {
						disconnected = true;
						report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
					}
					break;
				} else
					report(APIEvent::Type::FailedToWrite, APIEvent::Severity::EventWarning);
			} else
				offset += writeBytes;
		}
		
	}
}