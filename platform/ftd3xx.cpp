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
	setIsDisconnected(false);

	if(readThread.joinable())
		readThread.join();
	if(writeThread.joinable())
		writeThread.join();

	clearBuffers();

	if(const auto ret = FT_Close(*handle); ret != FT_OK) {
		addEvent(ret, APIEvent::Severity::EventWarning);
	}

	setIsClosing(false);

	return true;
}

void FTD3XX::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	static constexpr auto bufferSize = 2048;
	uint8_t buffer[bufferSize] = {};

	FT_SetStreamPipe(*handle, false, false, READ_PIPE_ID, bufferSize);
	FT_SetPipeTimeout(*handle, READ_PIPE_ID, 1);
	while(!isClosing() && !isDisconnected()) {
		ULONG received = 0;
		OVERLAPPED overlap = {};
		FT_InitializeOverlapped(*handle, &overlap);
		#ifdef _WIN32
			FT_ReadPipe(*handle, READ_PIPE_ID, buffer, bufferSize, &received, &overlap);
		#else
			FT_ReadPipeAsync(*handle, 0, buffer, bufferSize, &received, &overlap);
		#endif
		while(!isClosing()) {
			const auto ret = FT_GetOverlappedResult(*handle, &overlap, &received, true);
			if(ret == FT_IO_PENDING)
				continue;
			if(ret != FT_OK) {
				if(ret == FT_IO_ERROR) {
					setIsDisconnected(true);
					report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
				} else {
					addEvent(ret, APIEvent::Severity::Error);
				}
				FT_AbortPipe(*handle, READ_PIPE_ID);
			}
			break;
		}
		FT_ReleaseOverlapped(*handle, &overlap);
		if(received > 0) {
			pushRx(buffer, received);
		}
	}
}

void FTD3XX::writeTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	FT_SetPipeTimeout(*handle, WRITE_PIPE_ID, 100);
	WriteOperation writeOp;
	while(!isClosing() && !isDisconnected()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		const auto size = static_cast<ULONG>(writeOp.bytes.size());
		ULONG sent = 0;
		OVERLAPPED overlap = {};
		FT_InitializeOverlapped(*handle, &overlap);
		FT_SetStreamPipe(*handle, false, false, WRITE_PIPE_ID, size);
		#ifdef _WIN32
			FT_WritePipe(*handle, WRITE_PIPE_ID, writeOp.bytes.data(), size, &sent, &overlap);
		#else
			FT_WritePipeAsync(*handle, 0, writeOp.bytes.data(), size, &sent, &overlap);
		#endif
		while(!isClosing()) {
			const auto ret = FT_GetOverlappedResult(*handle, &overlap, &sent, true);
			if(ret == FT_IO_PENDING)
				continue;
			if(ret != FT_OK) {
				if(ret == FT_IO_ERROR) {
					setIsDisconnected(true);
					report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
				} else {
					addEvent(ret, APIEvent::Severity::Error);
				}
				FT_AbortPipe(*handle, WRITE_PIPE_ID);
			}
			break;
		}
		FT_ReleaseOverlapped(*handle, &overlap);
	}
}
