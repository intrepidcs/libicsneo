#include <vector>
#include "icsneo/api/eventmanager.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4091)
#endif
#define FTD3XX_STATIC
#include <ftd3xx.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "icsneo/platform/ftd3xx.h"

static constexpr auto READ_PIPE_ID = 0x82;
static constexpr auto WRITE_PIPE_ID = 0x02;

using namespace icsneo;

static void addEvent(FT_STATUS status, APIEvent::Severity severity) {
	const auto internalEvent = static_cast<uint32_t>(APIEvent::Type::FTOK) + status;
	EventManager::GetInstance().add(APIEvent((APIEvent::Type)internalEvent, severity));
}

void FTD3XX::Find(std::vector<FoundDevice>& found) {
	DWORD count;
	if(const auto ret = FT_CreateDeviceInfoList(&count); ret != FT_OK) {
		addEvent(ret, APIEvent::Severity::EventWarning);
		return;
	}
	if(count == 0) {
		return;
	}
	std::vector<FT_DEVICE_LIST_INFO_NODE> devices(count);
	if(const auto ret = FT_GetDeviceInfoList(devices.data(), &count); ret != FT_OK) {
		addEvent(ret, APIEvent::Severity::EventWarning);
		return;
	}
	for(const auto& dev : devices) {
		FoundDevice foundDevice = {};
		std::copy(dev.SerialNumber, dev.SerialNumber + sizeof(foundDevice.serial), foundDevice.serial);
		foundDevice.makeDriver = [](const device_eventhandler_t& eh, neodevice_t& forDevice) {
			return std::unique_ptr<Driver>(new FTD3XX(eh, forDevice));
		};
		found.push_back(std::move(foundDevice));
	}
}


FTD3XX::FTD3XX(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err), device(forDevice) {
}

bool FTD3XX::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	void* tmpHandle;
	if(const auto ret = FT_Create(device.serial, FT_OPEN_BY_SERIAL_NUMBER, &tmpHandle); ret != FT_OK) {
		addEvent(ret, APIEvent::Severity::Error);
		return false;
	}
	handle.emplace(tmpHandle);

	setIsClosing(false);
	setIsDisconnected(false);
	readThread = std::thread(&FTD3XX::readTask, this);
	writeThread = std::thread(&FTD3XX::writeTask, this);

	return true;
}

bool FTD3XX::isOpen() {
	return handle.has_value();
}

bool FTD3XX::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	setIsClosing(true);

	// unblock the read thread
	FT_AbortPipe(*handle, READ_PIPE_ID);

	if(readThread.joinable())
		readThread.join();
	if(writeThread.joinable())
		writeThread.join();

	clearBuffers();

	if(const auto ret = FT_Close(*handle); ret != FT_OK) {
		addEvent(ret, APIEvent::Severity::EventWarning);
	}

	handle.reset();

	setIsClosing(false);

	return true;
}

void FTD3XX::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	std::vector<uint8_t> buffer(2 * 1024 * 1024);

	FT_SetStreamPipe(*handle, false, false, READ_PIPE_ID, (ULONG)buffer.size());

	// disable timeouts, we will interupt the read thread with AbortPipe
	FT_SetPipeTimeout(*handle, READ_PIPE_ID, 0);

	OVERLAPPED overlapped = {};
	FT_InitializeOverlapped(*handle, &overlapped);

	FT_STATUS status;
	ULONG received = 0;

	while(!isClosing() && !isDisconnected()) {
		received = 0;
		#ifdef _WIN32
		status = FT_ReadPipe(*handle, READ_PIPE_ID, buffer.data(), (ULONG)buffer.size(), &received, &overlapped);
		#else
		status = FT_ReadPipeAsync(*handle, 0, buffer.data(), buffer.size(), &received, &overlapped);
		#endif
		if(FT_FAILED(status)) {
			if(status != FT_IO_PENDING) {
				addEvent(status, APIEvent::Severity::Error);
				setIsDisconnected(true);
				break;
			}
			status = FT_GetOverlappedResult(*handle, &overlapped, &received, true);
			if(FT_FAILED(status)) {
				addEvent(status, APIEvent::Severity::Error);
				setIsDisconnected(true);
				break;
			}
			if(received > 0) {
				pushRx(buffer.data(), received);
			}
		}
	}

	FT_ReleaseOverlapped(*handle, &overlapped);
}

void FTD3XX::writeTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	FT_SetPipeTimeout(*handle, WRITE_PIPE_ID, 0);
	WriteOperation writeOp;
	ULONG sent;
	FT_STATUS status;

	while(!isClosing() && !isDisconnected()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		const auto size = static_cast<ULONG>(writeOp.bytes.size());
		sent = 0;
		#ifdef _WIN32
		status = FT_WritePipe(*handle, WRITE_PIPE_ID, writeOp.bytes.data(), size, &sent, nullptr);
		#else
		status = FT_WritePipe(*handle, WRITE_PIPE_ID, writeOp.bytes.data(), size, &sent, 100);
		#endif
		if(FT_FAILED(status)) {
			addEvent(status, APIEvent::Severity::Error);
			setIsDisconnected(true);
			break;
		}
		if(sent != size) {
			report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
			setIsDisconnected(true);
			break;
		}
	}
}
