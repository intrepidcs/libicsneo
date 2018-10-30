#include "icsneo/platform/ftdi.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <memory>

using namespace icsneo;

// Instantiate static variables
neodevice_handle_t FTDI::handleCounter = 1;
Ftdi::Context FTDI::context;
std::vector<FTDI::FTDIDevice> FTDI::searchResultDevices;

/* Theory: Ftdi::List::find_all gives us back Ftdi::Context objects, but these can't be passed
 * back and forth with C nicely. So we wrap the Ftdi::Context objects in FTDIDevice classes which
 * will give it a nice neodevice_handle_t handle that we can reference it by. These FTDIDevice objects are
 * stored in searchResultDevices, and then moved into the instantiated FTDI class by the constructor.
 */
std::vector<neodevice_t> FTDI::FindByProduct(int product) {
	constexpr size_t deviceSerialBufferLength = sizeof(device.serial);
	std::vector<neodevice_t> found;

	auto devlist = std::unique_ptr<Ftdi::List>(Ftdi::List::find_all(context, INTREPID_USB_VENDOR_ID, product));
	searchResultDevices.clear();
	for(auto it = devlist->begin(); it != devlist->end(); it++)
		searchResultDevices.push_back(*it); // The upconversion to FTDIDevice will assign a handle

	for(auto& dev : searchResultDevices) {
		neodevice_t d;
		auto& serial = dev.serial();
		strncpy(d.serial, serial.c_str(), deviceSerialBufferLength - 1);
		d.serial[deviceSerialBufferLength - 1] = '\0'; // strncpy does not write a null terminator if serial is too long
		d.handle = dev.handle;
		found.push_back(d);
	}

	return found;
}

bool FTDI::IsHandleValid(neodevice_handle_t handle) {
	for(auto& dev : searchResultDevices) {
		if(dev.handle != handle)
			continue;

		return true;
	}
	return false;
}

bool FTDI::GetDeviceForHandle(neodevice_handle_t handle, FTDIDevice& device) {
	for(auto& dev : searchResultDevices) {
		if(dev.handle != handle)
			continue;
		
		device = dev;
		return true;
	}
	return false;
}

FTDI::FTDI(device_errorhandler_t err, neodevice_t& forDevice) : device(forDevice), err(err) {
	openable = GetDeviceForHandle(forDevice.handle, ftdiDevice);
}

bool FTDI::open() {
	if(isOpen() || !openable)
		return false;

	if(ftdiDevice.open())
		return false;

	ftdiDevice.set_usb_read_timeout(100);
	ftdiDevice.set_usb_write_timeout(1000);
	ftdiDevice.reset();
	ftdiDevice.set_baud_rate(500000);
	ftdiDevice.flush();

	// Create threads
	closing = false;
	readThread = std::thread(&FTDI::readTask, this);
	writeThread = std::thread(&FTDI::writeTask, this);

	return true;
}

bool FTDI::close() {
	if(!isOpen())
		return false;

	closing = true;
	
	if(readThread.joinable())
		readThread.join();
	
	if(writeThread.joinable())
		writeThread.join();

	if(ftdiDevice.close())
		return false;

	return true;
}

void FTDI::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 8;
	uint8_t readbuf[READ_BUFFER_SIZE];
	while(!closing) {
		auto readBytes = ftdiDevice.read(readbuf, READ_BUFFER_SIZE);
		if(readBytes > 0)
			readQueue.enqueue_bulk(readbuf, readBytes);
	}
}

void FTDI::writeTask() {
	WriteOperation writeOp;
	while(!closing) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		ftdiDevice.write(writeOp.bytes.data(), (int)writeOp.bytes.size());
	}
}