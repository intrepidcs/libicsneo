#include "libusb-1.0/libusb.h"
#include "icsneo/platform/ftdi.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <memory>
#include <utility>
#include <algorithm>

using namespace icsneo;

std::vector<std::tuple<int, std::string>> FTDI::handles;

std::vector<neodevice_t> FTDI::FindByProduct(int product) {
	constexpr size_t deviceSerialBufferLength = sizeof(device.serial);
	std::vector<neodevice_t> found;
	static FTDIContext context;

	std::pair<int, std::vector<std::string>> result = context.findDevices(product);
	if(result.first < 0)
		return found; // TODO Flag an error for the client application, there was an issue with FTDI

	for(auto& serial : result.second) {
		neodevice_t d;
		strncpy(d.serial, serial.c_str(), deviceSerialBufferLength - 1);
		d.serial[deviceSerialBufferLength - 1] = '\0'; // strncpy does not write a null terminator if serial is too long
		std::tuple<int, std::string> devHandle = std::make_tuple(product, serial);
		auto it = std::find(handles.begin(), handles.end(), devHandle);
		size_t foundHandle = SIZE_MAX;
		if(it != handles.end()) {
			foundHandle = it - handles.begin();
		} else {
			foundHandle = handles.size();
			handles.push_back(devHandle);
		}
		d.handle = foundHandle;
		found.push_back(d);
	}

	return found;
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
	std::tuple<int, std::string>& handle = handles[device.handle];
	const int openError = ftdi.openDevice(std::get<0>(handle), std::get<1>(handle).c_str());
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
	
	uint8_t flush;
	WriteOperation flushop;
	while(readQueue.try_dequeue(flush)) {}
	while(writeQueue.try_dequeue(flushop)) {}

	closing = false;
	return ret;
}

std::pair<int, std::vector<std::string>> FTDI::FTDIContext::findDevices(int pid) {
	std::pair<int, std::vector<std::string>> ret;
	
	if(context == nullptr) {
		ret.first = -1;
		return ret;
	}
	if(pid == 0) {
		ret.first = -2;
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

	for (struct ftdi_device_list* curdev = devlist; curdev != NULL;) {
		char serial[32];
		memset(serial, 0, sizeof(serial));
		int result = ftdi_usb_get_strings(context, curdev->dev, nullptr, 0, nullptr, 0, serial, 32);
		size_t len = strlen(serial);
		if(result >= 0 && len > 0)
			ret.second.emplace_back(serial);
		else if(ret.first > 0)
			ret.first--; // We're discarding this device
		curdev = curdev->next;
	}

	ftdi_list_free(&devlist);
	return ret;
}

int FTDI::FTDIContext::openDevice(int pid, const char* serial) {
	if(context == nullptr)
		return 1;
	if(pid == 0 || serial == nullptr)
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

void FTDI::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 8;
	uint8_t readbuf[READ_BUFFER_SIZE];
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing && !isDisconnected()) {
		auto readBytes = ftdi.read(readbuf, READ_BUFFER_SIZE);
		if(readBytes < 0) {
			if(readBytes == LIBUSB_ERROR_NO_DEVICE || readBytes == LIBUSB_ERROR_PIPE) {
				if(!isDisconnected()) {
					disconnected = true;
					report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
				}
			} else
				report(APIEvent::Type::FailedToRead, APIEvent::Severity::EventWarning);
		} else
			readQueue.enqueue_bulk(readbuf, readBytes);
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
				if(writeBytes == LIBUSB_ERROR_NO_DEVICE || writeBytes == LIBUSB_ERROR_PIPE) {
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