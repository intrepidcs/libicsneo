#include "icsneo/platform/windows/ftdi.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/platform/registry.h"
#include "icsneo/device/founddevice.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cwctype>
#include <algorithm>
#include <codecvt>
#include <limits>
#include <stdio.h>

using namespace icsneo;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
static const std::wstring DRIVER_SERVICES_REG_KEY = L"SYSTEM\\CurrentControlSet\\services\\";
static const std::wstring ALL_ENUM_REG_KEY = L"SYSTEM\\CurrentControlSet\\Enum\\";
static constexpr unsigned int RETRY_TIMES = 5;
static constexpr unsigned int RETRY_DELAY = 50;

struct VCP::Detail {
	Detail() {
		overlappedRead.hEvent = INVALID_HANDLE_VALUE;
		overlappedWrite.hEvent = INVALID_HANDLE_VALUE;
		overlappedWait.hEvent = INVALID_HANDLE_VALUE;
	}
	HANDLE handle = INVALID_HANDLE_VALUE;
	OVERLAPPED overlappedRead = {};
	OVERLAPPED overlappedWrite = {};
	OVERLAPPED overlappedWait = {};
};

void VCP::Find(std::vector<FoundDevice>& found, std::vector<std::wstring> driverNames) {
	for(auto& driverName : driverNames) {
		std::wstringstream regss;
		regss << DRIVER_SERVICES_REG_KEY << driverName << L"\\Enum\\";
		std::wstring driverEnumRegKey = regss.str();

		uint32_t deviceCount = 0;
		if(!Registry::Get(driverEnumRegKey, L"Count", deviceCount))
			continue;

		for(uint32_t i = 0; i < deviceCount; i++) {
			FoundDevice device;

			device.makeDriver = [](const device_eventhandler_t& reportFn, neodevice_t& device) {
				return std::unique_ptr<Driver>(new VCP(reportFn, device));
			};

			// First we want to look at what devices FTDI is enumerating (inside driverEnumRegKey)
			// The entry for a ValueCAN 3 with SN 138635 looks like "FTDIBUS\VID_093C+PID_0601+138635A\0000"
			// The entry for a ValueCAN 4 with SN V20227 looks like "USB\VID_093C&PID_1101\V20227"
			std::wstringstream ss;
			ss << i;
			std::wstring entry;
			if(!Registry::Get(driverEnumRegKey, ss.str(), entry))
				continue;

			std::transform(entry.begin(), entry.end(), entry.begin(), std::towupper);

			std::wstringstream vss;
			vss << "VID_" << std::setfill(L'0') << std::setw(4) << std::uppercase << std::hex << INTREPID_USB_VENDOR_ID; // Intrepid Vendor ID
			if(entry.find(vss.str()) == std::wstring::npos)
				continue;

			auto pidpos = entry.find(L"PID_");
			if(pidpos == std::wstring::npos)
				continue;
			// We will later use this and startchar to parse the PID

			// Okay, this is a device we want
			// Get the serial number
			auto startchar = entry.find(L"+", pidpos + 1);
			if(startchar == std::wstring::npos)
				startchar = entry.find(L"\\", pidpos + 1);
			bool conversionError = false;
			int sn = 0;
			try {
				sn = std::stoi(entry.substr(startchar + 1));
			}
			catch(...) {
				conversionError = true;
			}

			std::wstringstream oss;
			if(!sn || conversionError)
				oss << entry.substr(startchar + 1, 6); // This is a device with characters in the serial number
			else
				oss << sn;

			device.productId = uint16_t(std::wcstol(entry.c_str() + pidpos + 4, nullptr, 16));
			if(!device.productId)
				continue;

			std::string serial = converter.to_bytes(oss.str());
			// The serial number should not have a path slash in it. If it does, that means we don't have the real serial.
			if(serial.find_first_of('\\') != std::string::npos) {
				// The serial number was not in the first serenum key where we expected it.
				// We can try to match the ContainerID with the one in ALL_ENUM\USB and get a serial that way
				std::wstringstream uess;
				uess << ALL_ENUM_REG_KEY << L"\\USB\\" << vss.str() << L"&PID_" << std::setfill(L'0') << std::setw(4)
					<< std::uppercase << std::hex << device.productId << L'\\';
				std::wstringstream ciss;
				ciss << ALL_ENUM_REG_KEY << entry;
				std::wstring containerIDFromEntry, containerIDFromEnum;
				if(!Registry::Get(ciss.str(), L"ContainerID", containerIDFromEntry))
					continue; // We did not get a container ID. This can happen on Windows XP and before.
				if(containerIDFromEntry.empty())
					continue; // The container ID was empty?
				std::vector<std::wstring> subkeys;
				if(!Registry::EnumerateSubkeys(uess.str(), subkeys))
					continue; // VID/PID combo was not present at all.
				if(subkeys.empty())
					continue; // No devices for VID/PID.
				std::wstring correctSerial;
				for(auto& subkey : subkeys) {
					std::wstringstream skss;
					skss << uess.str() << L'\\' << subkey;
					if(!Registry::Get(skss.str(), L"ContainerID", containerIDFromEnum))
						continue;
					if(containerIDFromEntry != containerIDFromEnum)
						continue;
					correctSerial = subkey;
					break;
				}
				if(correctSerial.empty())
					continue; // Didn't find the device within the subkeys of the enumeration

				sn = 0;
				conversionError = false;
				try {
					sn = std::stoi(correctSerial);
				}
				catch(...) {
					conversionError = true;
				}

				if(!sn || conversionError) {
					// This is a device with characters in the serial number
					if(correctSerial.size() != 6)
						continue;
					serial = converter.to_bytes(correctSerial);
				}
				else {
					std::wstringstream soss;
					soss << sn;
					serial = converter.to_bytes(soss.str());
				}

				if(serial.find_first_of('\\') != std::string::npos)
					continue;
			}
			strcpy_s(device.serial, sizeof(device.serial), serial.c_str());

			// Serial number is saved, we want the COM port number now
			// This will be stored under ALL_ENUM_REG_KEY\entry\Device Parameters\PortName (entry from the FTDI_ENUM)
			std::wstringstream dpss;
			dpss << ALL_ENUM_REG_KEY << entry << L"\\Device Parameters";
			std::wstring port;
			Registry::Get(dpss.str(), L"PortName", port); // TODO If error do something else (Plasma maybe?)
			std::transform(port.begin(), port.end(), port.begin(), std::towupper);
			auto compos = port.find(L"COM");
			device.handle = 0;
			if(compos != std::wstring::npos) {
				try {
					device.handle = std::stoi(port.substr(compos + 3));
				}
				catch(...) {} // In case of this, or any other error, handle has already been initialized to 0
			}

			bool alreadyFound = false;
			FoundDevice* shouldReplace = nullptr;
			for(auto& foundDev : found) {
				if((foundDev.handle == device.handle || foundDev.handle == 0 || device.handle == 0) && serial == foundDev.serial) {
					alreadyFound = true;
					if(foundDev.handle == 0)
						shouldReplace = &foundDev;
					break;
				}
			}

			if(!alreadyFound)
				found.push_back(device);
			else if(shouldReplace != nullptr)
				*shouldReplace = device;
		}
	}
}

VCP::VCP(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err), device(forDevice) {
	detail = std::make_shared<Detail>();
}

bool VCP::IsHandleValid(neodevice_handle_t handle) {
	if(handle < 1)
		return false;

	if(handle > 256) // Windows default max COM port is COM256
		return false; // TODO Enumerate subkeys of HKLM\HARDWARE\DEVICEMAP\SERIALCOMM as a user might have more serial ports somehow

	return true;
}

bool VCP::open(bool fromAsync) {
	if(isOpen() || (!fromAsync && opening)) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	if(!IsHandleValid(device.handle)) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	opening = true;

	std::wstringstream comss;
	comss << L"\\\\.\\COM" << device.handle;

	// We're going to attempt to open 5 (RETRY_TIMES) times in a row
	for(int i = 0; !isOpen() && i < RETRY_TIMES; i++) {
		detail->handle = CreateFileW(comss.str().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		if(GetLastError() == ERROR_SUCCESS)
			break; // We have the file handle

		std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
	}

	opening = false;

	if(!isOpen()) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Set the timeouts
	COMMTIMEOUTS timeouts;
	if(!GetCommTimeouts(detail->handle, &timeouts)) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// See https://docs.microsoft.com/en-us/windows/desktop/api/winbase/ns-winbase-_commtimeouts#remarks
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutConstant = 10000;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	if(!SetCommTimeouts(detail->handle, &timeouts)) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Set the COM state
	DCB comstate;
	if(!GetCommState(detail->handle, &comstate)) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	comstate.BaudRate = 115200;
	comstate.ByteSize = 8;
	comstate.Parity = NOPARITY;
	comstate.StopBits = 0;
	comstate.fDtrControl = DTR_CONTROL_ENABLE;
	comstate.fRtsControl = RTS_CONTROL_ENABLE;

	if(!SetCommState(detail->handle, &comstate)) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	PurgeComm(detail->handle, PURGE_RXCLEAR);

	// Set up events so that overlapped IO can work with them
	detail->overlappedRead.hEvent = CreateEvent(nullptr, false, false, nullptr);
	detail->overlappedWrite.hEvent = CreateEvent(nullptr, false, false, nullptr);
	detail->overlappedWait.hEvent = CreateEvent(nullptr, true, false, nullptr);
	if (detail->overlappedRead.hEvent == nullptr || detail->overlappedWrite.hEvent == nullptr || detail->overlappedWait.hEvent == nullptr) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Set up event so that we will satisfy overlappedWait when a character comes in
	if(!SetCommMask(detail->handle, EV_RXCHAR)) {
		close();
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	
	// TODO Set up some sort of shared memory, save which COM port we have open so we don't try to open it again
	
	// Create threads
	readThread = std::thread(&VCP::readTask, this);
	writeThread = std::thread(&VCP::writeTask, this);

	return true;
}

void VCP::openAsync(fn_boolCallback callback) {
	threads.push_back(std::make_shared<std::thread>([&]() {
		callback(open(true));
	}));
}

bool VCP::close() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}
		
	closing = true; // Signal the threads that we are closing
	for(auto& t : threads)
		t->join(); // Wait for the threads to close
	readThread.join();
	writeThread.join();
	closing = false;

	if(!CloseHandle(detail->handle)) {
		report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
		return false;
	}
		
	detail->handle = INVALID_HANDLE_VALUE;

	bool ret = true; // If one of the events fails closing, we probably still want to try and close the others
	if(detail->overlappedRead.hEvent != INVALID_HANDLE_VALUE) {
		if(!CloseHandle(detail->overlappedRead.hEvent))
			ret = false;
		detail->overlappedRead.hEvent = INVALID_HANDLE_VALUE;
	}
	if(detail->overlappedWrite.hEvent != INVALID_HANDLE_VALUE) {
		if(!CloseHandle(detail->overlappedWrite.hEvent))
			ret = false;
		detail->overlappedWrite.hEvent = INVALID_HANDLE_VALUE;
	}
	if(detail->overlappedWait.hEvent != INVALID_HANDLE_VALUE) {
		if(!CloseHandle(detail->overlappedWait.hEvent))
			ret = false;
		detail->overlappedWait.hEvent = INVALID_HANDLE_VALUE;
	}

	uint8_t flush;
	WriteOperation flushop;
	while(readQueue.try_dequeue(flush)) {}
	while(writeQueue.try_dequeue(flushop)) {}

	if(!ret)
		report(APIEvent::Type::DriverFailedToClose, APIEvent::Severity::Error);
	
	// TODO Set up some sort of shared memory, free which COM port we had open so we can try to open it again

	return ret;
}

bool VCP::isOpen() {
	return detail->handle != INVALID_HANDLE_VALUE;
}

void VCP::readTask() {
	constexpr size_t READ_BUFFER_SIZE = 10240;
	uint8_t readbuf[READ_BUFFER_SIZE];
	IOTaskState state = LAUNCH;
	DWORD bytesRead = 0;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing && !isDisconnected()) {
		switch(state) {
			case LAUNCH: {
				COMSTAT comStatus;
				unsigned long errorCodes;
				ClearCommError(detail->handle, &errorCodes, &comStatus);

				bytesRead = 0;
				if(ReadFile(detail->handle, readbuf, READ_BUFFER_SIZE, nullptr, &detail->overlappedRead)) {
					if(GetOverlappedResult(detail->handle, &detail->overlappedRead, &bytesRead, FALSE)) {
						if(bytesRead)
							readQueue.enqueue_bulk(readbuf, bytesRead);
					}
					continue;
				}

				auto lastError = GetLastError();
				if(lastError == ERROR_IO_PENDING)
					state = WAIT;
				else if(lastError != ERROR_SUCCESS) {
					if(lastError == ERROR_ACCESS_DENIED) {
						if(!isDisconnected()) {
							disconnected = true;
							report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
						}
					} else
						report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
				}
			}
			break;
			case WAIT: {
				auto ret = WaitForSingleObject(detail->overlappedRead.hEvent, 100);
				if(ret == WAIT_OBJECT_0) {
					if(GetOverlappedResult(detail->handle, &detail->overlappedRead, &bytesRead, FALSE)) {
						readQueue.enqueue_bulk(readbuf, bytesRead);
						state = LAUNCH;
					} else
						report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
				}
				if(ret == WAIT_ABANDONED || ret == WAIT_FAILED) {
					state = LAUNCH;
					report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
				}
			}
		}
	}
}

void VCP::writeTask() {
	IOTaskState state = LAUNCH;
	VCP::WriteOperation writeOp;
	DWORD bytesWritten = 0;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing && !isDisconnected()) {
		switch(state) {
			case LAUNCH: {
				if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
					continue;

				bytesWritten = 0;
				if(WriteFile(detail->handle, writeOp.bytes.data(), (DWORD)writeOp.bytes.size(), nullptr, &detail->overlappedWrite))
					continue;
				
				auto winerr = GetLastError();
				if(winerr == ERROR_IO_PENDING) {
					state = WAIT;
				}
				else if(winerr == ERROR_ACCESS_DENIED) {
					if(!isDisconnected()) {
						disconnected = true;
						report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
					}
				} else
					report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
			}
			break;
			case WAIT: {
				auto ret = WaitForSingleObject(detail->overlappedWrite.hEvent, 50);
				if(ret == WAIT_OBJECT_0) {
					if(!GetOverlappedResult(detail->handle, &detail->overlappedWrite, &bytesWritten, FALSE))
						report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
					state = LAUNCH;
				}
				
				if(ret == WAIT_ABANDONED) {
					report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
					state = LAUNCH;
				}
			}
		}
	}
}