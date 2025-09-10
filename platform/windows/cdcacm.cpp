#include "icsneo/platform/windows/cdcacm.h"

#include <setupapi.h>
#include <initguid.h>
#include <usbiodef.h>
#include <devpkey.h>

using namespace icsneo;

#define SERIAL_SIZE 6

CDCACM::CDCACM(const device_eventhandler_t& err, const std::wstring& path) : Driver(err), path(path) {
}

bool CDCACM::open() {
	handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
	if(handle == INVALID_HANDLE_VALUE) {
		EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
		return false;
	}
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = MAXDWORD - 1;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	if(!SetCommTimeouts(handle, &timeouts)) {
		EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		return false;
	}

	DCB comstate;
	if(!GetCommState(handle, &comstate)) {
		EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		return false;
	}
	comstate.BaudRate = 115200;
	comstate.ByteSize = 8;
	comstate.fRtsControl = RTS_CONTROL_DISABLE;
	if(!SetCommState(handle, &comstate)) {
		EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		return false;
	}

	PurgeComm(handle, PURGE_RXCLEAR);

	readOverlapped.hEvent = CreateEventA(nullptr, false, false, nullptr);
	writeOverlapped.hEvent = CreateEventA(nullptr, false, false, nullptr);

	setIsDisconnected(false);
    readThread = std::thread(&CDCACM::read, this);
    writeThread = std::thread(&CDCACM::write, this);
	return true;
}

bool CDCACM::isOpen() {
	return handle != INVALID_HANDLE_VALUE;
}

bool CDCACM::close() {
	setIsClosing(true);
	SetEvent(readOverlapped.hEvent); // unblock read thread
	SetEvent(writeOverlapped.hEvent); // unblock write thread if waiting on COM write
	writeQueue.enqueue(WriteOperation{}); // unblock write thread if waiting on write queue pop
	readThread.join();
    writeThread.join();
	CloseHandle(readOverlapped.hEvent);
	CloseHandle(writeOverlapped.hEvent);
	CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
	setIsClosing(false);
	return true;
}

void CDCACM::read() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	std::vector<uint8_t> buffer(ICSNEO_DRIVER_RINGBUFFER_SIZE);

	while(!isDisconnected() && !isClosing()) {
		if(!ReadFile(handle, buffer.data(), (DWORD)buffer.size(), nullptr, &readOverlapped)) {
			if(GetLastError() != ERROR_IO_PENDING) {
				EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
				setIsDisconnected(true);
				return;
			}
		}
		DWORD read = 0;
		if(!GetOverlappedResult(handle, &readOverlapped, &read, true)) {
			EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		if(read == 0) {
			continue;
		}
		while(!isDisconnected() && !isClosing()) {
			if(pushRx(buffer.data(), read))
				break;
		}
	}
}

void CDCACM::write() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	WriteOperation writeOp;

	while(!isDisconnected() && !isClosing()) {
		writeQueue.wait_dequeue(writeOp);

		if(isDisconnected() || isClosing()) {
            return;
        }

		if(!WriteFile(handle, writeOp.bytes.data(), (DWORD)writeOp.bytes.size(), nullptr, &writeOverlapped)) {
			if(GetLastError() != ERROR_IO_PENDING) {
				EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
				setIsDisconnected(true);
				return;
			}
		}
		DWORD written;
		if(!GetOverlappedResult(handle, &writeOverlapped, &written, true)) {
			EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}

		if(written != writeOp.bytes.size()) {
			EventManager::GetInstance().add(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
	}
}


class DeviceInfo {
public:
	DeviceInfo() {
		mDeviceInfo = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_USB_DEVICE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	}
	~DeviceInfo() {
		SetupDiDestroyDeviceInfoList(mDeviceInfo);
	}
	operator HDEVINFO() const {
		return mDeviceInfo;
	}
	operator bool() const {
		return mDeviceInfo != INVALID_HANDLE_VALUE;
	}
private:
	HDEVINFO mDeviceInfo;
};

class DeviceInfoData {
public:
	DeviceInfoData() {
		mDeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	}
	operator SP_DEVINFO_DATA*() {
		return &mDeviceInfoData;
	}
private:
	SP_DEVINFO_DATA mDeviceInfoData;
};

static constexpr size_t WSTRING_ELEMENT_SIZE = sizeof(std::wstring::value_type);

void CDCACM::Find(std::vector<FoundDevice>& found) {
	DeviceInfoData deviceInfoData;
	const std::wstring intrepidUSB(L"USB\\VID_093C");
	DeviceInfo deviceInfoSet;
	if(!deviceInfoSet) {
		EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
		return;
	}

	for(DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, deviceInfoData); ++i) {
		DWORD DataT;
		DWORD buffersize = 0;

		std::wstring wclass;
		while(!SetupDiGetDevicePropertyW(deviceInfoSet, deviceInfoData, &DEVPKEY_Device_Class, &DataT, reinterpret_cast<PBYTE>(wclass.data()), static_cast<DWORD>((wclass.size() + 1) * WSTRING_ELEMENT_SIZE), &buffersize, 0)) {
			wclass.resize((buffersize - 1) / WSTRING_ELEMENT_SIZE);
		}

		if(wclass != L"Ports") {
			continue;
		}

		// TODO: is this a bug in Windows? why is this returned size different/wrong? It's like it's not a wstring at all
		std::wstring deviceInstanceId;
		while(!SetupDiGetDeviceInstanceIdW(deviceInfoSet, deviceInfoData, deviceInstanceId.data(), static_cast<DWORD>(deviceInstanceId.size() + 1), &buffersize)) {
			deviceInstanceId.resize(buffersize - 1);
		}

		if(deviceInstanceId.find(intrepidUSB) != 0) {
			continue;
		}

		auto serialOffset = deviceInstanceId.find_last_of('\\');

		if(serialOffset == std::string::npos) {
			continue;
		}

		++serialOffset; // move past '\'

		if(deviceInstanceId.size() < serialOffset + SERIAL_SIZE) {
			continue;
		}

		std::wstring wserial(deviceInstanceId.c_str() + serialOffset, SERIAL_SIZE);

		FoundDevice device = {0};

		if(WideCharToMultiByte(CP_ACP, 0, wserial.c_str(), (int)wserial.size(), device.serial, SERIAL_SIZE, NULL, NULL) == 0) {
			EventManager::GetInstance().add(APIEvent::Type::SyscallError, APIEvent::Severity::Error);
			continue;
		}

		std::wstring wport;
		while(!SetupDiGetCustomDevicePropertyW(deviceInfoSet, deviceInfoData, L"PortName", 0, &DataT, reinterpret_cast<PBYTE>(wport.data()), static_cast<DWORD>((wport.size() + 1) * WSTRING_ELEMENT_SIZE), &buffersize)) {
			wport.resize((buffersize - 1) / WSTRING_ELEMENT_SIZE);
		}

		const std::wstring path(L"\\\\.\\" + wport);

		device.makeDriver = [path](device_eventhandler_t err, neodevice_t&) {
			return std::make_unique<CDCACM>(err, path);
		};

		found.emplace_back(std::move(device));
	}
}
