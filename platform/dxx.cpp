#include "icsneo/platform/dxx.h"

#define ICS_USB_VID 0x093C

using namespace icsneo;

static APIEvent::Type eventError(libredxx_status status) {
	switch (status) {
		case LIBREDXX_STATUS_ERROR_SYS: return APIEvent::Type::DXXErrorSys;
		case LIBREDXX_STATUS_ERROR_INTERRUPTED: return APIEvent::Type::DXXErrorSys;
		case LIBREDXX_STATUS_ERROR_OVERFLOW: return APIEvent::Type::DXXErrorSys;
		case LIBREDXX_STATUS_ERROR_IO: return APIEvent::Type::DXXErrorSys;
		case LIBREDXX_STATUS_ERROR_INVALID_ARGUMENT: return APIEvent::Type::DXXErrorSys;
		default: return APIEvent::Type::Unknown;
	}
}

void DXX::Find(std::vector<FoundDevice>& found) {
	libredxx_status status;
	static libredxx_find_filter filters[] = {
		{ LIBREDXX_DEVICE_TYPE_D2XX, { ICS_USB_VID, 0x0005 } }, // RAD-Star 2
		{ LIBREDXX_DEVICE_TYPE_D2XX, { ICS_USB_VID, 0x0006 } }, // RAD-A2B Rev A
		{ LIBREDXX_DEVICE_TYPE_D2XX, { ICS_USB_VID, 0x1000 } }, // neoVI FIRE2
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1201 } }, // RAD-SuperMoon
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1202 } }, // RAD-Moon2
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1203 } }, // RAD-Gigalog
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1204 } }, // RAD-Gigastar
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1206 } }, // RAD-A2B Rev B
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1207 } }, // RAD-Comet
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1208 } }, // RAD-Comet3
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1209 } }, // RAD-MoonT1S
		{ LIBREDXX_DEVICE_TYPE_D3XX, { ICS_USB_VID, 0x1210 } }, // RAD-Gigastar 2
	};
	static size_t filterCount = sizeof(filters) / sizeof(filters[0]);

	libredxx_found_device** foundDevices = nullptr;
	size_t foundDevicesCount;
	status = libredxx_find_devices(filters, filterCount, &foundDevices, &foundDevicesCount);
	if(status != LIBREDXX_STATUS_SUCCESS) {
		EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
		return;
	}
	if(foundDevicesCount == 0) {
		return;
	}
	for(size_t i = 0; i < foundDevicesCount; ++i) {
		libredxx_found_device* foundDevice = foundDevices[i];
		libredxx_serial serial = {};
		status = libredxx_get_serial(foundDevice, &serial);
		if(status != LIBREDXX_STATUS_SUCCESS) {
			EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
			continue;
		}

		libredxx_device_id id;
		status = libredxx_get_device_id(foundDevice, &id);
		if(status != LIBREDXX_STATUS_SUCCESS) {
			EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
			continue;
		}

		libredxx_device_type type;
		status = libredxx_get_device_type(foundDevice, &type);
		if(status != LIBREDXX_STATUS_SUCCESS) {
			EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
			continue;
		}

		auto& device = found.emplace_back();
		std::copy(serial.serial, serial.serial + sizeof(device.serial), device.serial);
		device.makeDriver = [id, type](device_eventhandler_t err, neodevice_t& forDevice) {
			return std::make_unique<DXX>(err, forDevice, id.pid, type);
		};
	}
	libredxx_free_found(foundDevices);
}

DXX::DXX(const device_eventhandler_t& err, neodevice_t& forDevice, uint16_t pid, libredxx_device_type type) :
	Driver(err), neodevice(forDevice), pid(pid), type(type) {
}

bool DXX::open() {
	libredxx_status status;
	libredxx_find_filter filters[] = {
		{ (libredxx_device_type)type, { ICS_USB_VID, pid } }
	};
	libredxx_found_device** foundDevices = nullptr;
	size_t foundDevicesCount;
	status = libredxx_find_devices(filters, 1, &foundDevices, &foundDevicesCount);
	if(status != LIBREDXX_STATUS_SUCCESS) {
		EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
		return false;
	}
	if(foundDevicesCount == 0) {
		EventManager::GetInstance().add(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
		return false;
	}

	libredxx_found_device* foundDevice = nullptr;
	for(size_t i = 0; i < foundDevicesCount; ++i) {
		libredxx_serial serial = {};
		status = libredxx_get_serial(foundDevices[i], &serial);
		if(status != LIBREDXX_STATUS_SUCCESS) {
			EventManager::GetInstance().add(eventError(status), APIEvent::Severity::EventWarning);
			continue;
		}
		if(strcmp(serial.serial, neodevice.serial) == 0) {
			foundDevice = foundDevices[i];
			break;
		}
	}
	if(foundDevice == nullptr) {
		EventManager::GetInstance().add(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
		libredxx_free_found(foundDevices);
		return false;
	}
	status = libredxx_open_device(foundDevice, &device);
	if(status != LIBREDXX_STATUS_SUCCESS) {
		EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
		libredxx_free_found(foundDevices);
		return false;
	}
	libredxx_free_found(foundDevices);
	setIsDisconnected(false);
	readThread = std::thread(&DXX::read, this);
	writeThread = std::thread(&DXX::write, this);
	return true;
}

bool DXX::isOpen() {
	return device != nullptr;
}

bool DXX::close() {
	setIsClosing(true);
	libredxx_close_device(device); // unblock read thread & close
	writeQueue.enqueue(WriteOperation{}); // unblock write thread
	readThread.join();
	writeThread.join();
	device = nullptr;
	setIsClosing(false);
	return true;
}

void DXX::read() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	std::vector<uint8_t> buffer(ICSNEO_DRIVER_RINGBUFFER_SIZE);

	while(!isDisconnected() && !isClosing()) {
		size_t received = buffer.size();
		const auto status = libredxx_read(device, buffer.data(), &received);
		if(isDisconnected() || isClosing()) {
			return;
		}
		if(status != LIBREDXX_STATUS_SUCCESS) {
			EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		while(!isDisconnected() && !isClosing()) {
			if(pushRx(buffer.data(), received))
				break;
		}
	}
}

void DXX::write() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	WriteOperation writeOp;

	while(!isDisconnected() && !isClosing()) {
		writeQueue.wait_dequeue(writeOp);

		if(isDisconnected() || isClosing()) {
			return;
		}

		for(size_t totalWritten = 0; totalWritten < writeOp.bytes.size();) {
			size_t size = writeOp.bytes.size() - totalWritten;
			const auto status = libredxx_write(device, &writeOp.bytes[totalWritten], &size);
			if(isDisconnected() || isClosing()) {
				return;
			}
			if(status != LIBREDXX_STATUS_SUCCESS) {
				EventManager::GetInstance().add(eventError(status), APIEvent::Severity::Error);
				setIsDisconnected(true);
				return;
			}
			totalWritten += size;
		}
	}
}
