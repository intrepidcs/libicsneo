#include "icsneo/api/event.h"
#include "icsneo/device/device.h"
#include <sstream>

using namespace icsneo;

APIEvent::APIEvent(Type type, Severity severity, const Device* device) : eventStruct({}) {
	this->device = device;
	if(device) {
		serial = device->getSerial();
		eventStruct.serial[serial.copy(eventStruct.serial, sizeof(eventStruct.serial))] = '\0';
	}

	init(type, severity);
}

APIEvent::APIEvent(neosocketevent_t evStruct, const Device* device)
{
	this->device = device;
	timepoint = EventClock::from_time_t(evStruct.timestamp);
	serial = std::string(evStruct.serial);
	eventStruct.eventNumber = evStruct.eventNumber;
	eventStruct.severity = evStruct.severity;
	eventStruct.description = DescriptionForType(APIEvent::getType());
	eventStruct.timestamp = evStruct.timestamp;
}

void APIEvent::init(Type event, APIEvent::Severity severity) {
	timepoint = EventClock::now();
	eventStruct.description = DescriptionForType(event);
	eventStruct.eventNumber = (uint32_t)event;
	eventStruct.severity = (uint8_t) severity;
	eventStruct.timestamp = EventClock::to_time_t(timepoint);
}

std::string APIEvent::describe() const noexcept {
	std::stringstream ss;
	if(device)
		ss << *device; // Makes use of device.describe()
	else
		ss << "API";
	
	Severity severity = getSeverity();
	if(severity == Severity::EventInfo) {
		ss << " Info: ";
	} else if(severity == Severity::EventWarning) {
		ss << " Warning: ";
	} else if(severity == Severity::Error) {
		ss << " Error: ";
	} else {
		// Should never get here, since Severity::Any should only be used for filtering
		ss << " Any: ";
	}

	ss << getDescription();
	return ss.str();
}

neosocketevent_t APIEvent::getNeoSocketEvent() const noexcept {
	neosocketevent_t neoSocketEvent;
	neoSocketEvent.eventNumber = eventStruct.eventNumber;
	neoSocketEvent.severity = eventStruct.severity;
	std::memcpy(neoSocketEvent.serial, eventStruct.serial, sizeof(eventStruct.serial));
	neoSocketEvent.timestamp = eventStruct.timestamp;
	return neoSocketEvent;
}

void APIEvent::downgradeFromError() noexcept {
	eventStruct.severity = (uint8_t) APIEvent::Severity::EventWarning;
}

bool APIEvent::isForDevice(std::string filterSerial) const noexcept {
	if(!device || filterSerial.length() == 0)
		return false;
	return device->getSerial() == filterSerial;
}

// API Errors
static constexpr const char* INVALID_NEODEVICE = "The provided neodevice_t object was invalid.";
static constexpr const char* REQUIRED_PARAMETER_NULL = "A required parameter was NULL.";
static constexpr const char* OUTPUT_TRUNCATED = "The output was too large for the provided buffer and has been truncated.";
static constexpr const char* BUFFER_INSUFFICIENT = "The provided buffer was insufficient. No data was written.";
static constexpr const char* PARAMETER_OUT_OF_RANGE = "A parameter was out of range.";
static constexpr const char* DEVICE_CURRENTLY_OPEN = "The device is currently open.";
static constexpr const char* DEVICE_CURRENTLY_CLOSED = "The device is currently closed.";
static constexpr const char* DEVICE_CURRENTLY_ONLINE = "The device is currently online.";
static constexpr const char* DEVICE_CURRENTLY_OFFLINE = "The device is currently offline.";
static constexpr const char* DEVICE_CURRENTLY_POLLING = "The device is currently polling for messages.";
static constexpr const char* DEVICE_NOT_CURRENTLY_POLLING = "The device is not currently polling for messages.";
static constexpr const char* UNSUPPORTED_TX_NETWORK = "Message network is not a supported TX network.";
static constexpr const char* MESSAGE_MAX_LENGTH_EXCEEDED = "The message was too long.";
static constexpr const char* VALUE_NOT_YET_PRESENT = "The value is not yet present.";
static constexpr const char* TIMEOUT = "The timeout was reached.";
static constexpr const char* WIVI_NOT_SUPPORTED = "Wireless neoVI functions are not supported on this device.";

// Device Errors
static constexpr const char* POLLING_MESSAGE_OVERFLOW = "Too many messages have been recieved for the polling message buffer, some have been lost!";
static constexpr const char* NO_SERIAL_NUMBER_FW_12V = "Communication could not be established with the device. Perhaps it is not powered with 12 volts?";
static constexpr const char* NO_SERIAL_NUMBER_FW = "Communication could not be established with the device. Perhaps it is not powered?";
static constexpr const char* NO_SERIAL_NUMBER_12V = "Communication could not be established with the device. Perhaps it is not powered with 12 volts or requires a firmware update using Vehicle Spy.";
static constexpr const char* NO_SERIAL_NUMBER = "Communication could not be established with the device. Perhaps it is not powered or requires a firmware update using Vehicle Spy.";
static constexpr const char* INCORRECT_SERIAL_NUMBER = "The device did not return the expected serial number!";
static constexpr const char* SETTINGS_READ = "The device settings could not be read.";
static constexpr const char* SETTINGS_VERSION = "The settings version is incorrect, please update your firmware with neoVI Explorer.";
static constexpr const char* SETTINGS_LENGTH = "The settings length is incorrect, please update your firmware with neoVI Explorer.";
static constexpr const char* SETTINGS_CHECKSUM = "The settings checksum is incorrect, attempting to set defaults may remedy this issue.";
static constexpr const char* SETTINGS_NOT_AVAILABLE = "Settings are not available for this device.";
static constexpr const char* SETTINGS_READONLY = "Settings are read-only for this device.";
static constexpr const char* CAN_SETTINGS_NOT_AVAILABLE = "CAN settings are not available for this device.";
static constexpr const char* CANFD_SETTINGS_NOT_AVAILABLE = "CANFD settings are not available for this device.";
static constexpr const char* LSFTCAN_SETTINGS_NOT_AVAILABLE = "LSFTCAN settings are not available for this device.";
static constexpr const char* SWCAN_SETTINGS_NOT_AVAILABLE = "SWCAN settings are not available for this device.";
static constexpr const char* BAUDRATE_NOT_FOUND = "The baudrate was not found.";
static constexpr const char* UNEXPECTED_NETWORK_TYPE = "The network type was not found.";
static constexpr const char* DEVICE_FIRMWARE_OUT_OF_DATE = "The device firmware is out of date. New API functionality may not be supported.";
static constexpr const char* SETTINGS_STRUCTURE_MISMATCH = "Unexpected settings structure for this device.";
static constexpr const char* SETTINGS_STRUCTURE_TRUNCATED = "Settings structure is longer than the device supports and will be truncated.";
static constexpr const char* NO_DEVICE_RESPONSE = "Expected a response from the device but none were found.";
static constexpr const char* MESSAGE_FORMATTING = "The message was not properly formed.";
static constexpr const char* CANFD_NOT_SUPPORTED = "This device does not support CANFD.";
static constexpr const char* RTR_NOT_SUPPORTED = "RTR is not supported with CANFD.";
static constexpr const char* DEVICE_DISCONNECTED = "The device was disconnected.";
static constexpr const char* ONLINE_NOT_SUPPORTED = "This device does not support going online.";
static constexpr const char* TERMINATION_NOT_SUPPORTED_DEVICE = "This device does not support software selectable termination.";
static constexpr const char* TERMINATION_NOT_SUPPORTED_NETWORK = "This network does not support software selectable termination on this device.";
static constexpr const char* ANOTHER_IN_TERMINATION_GROUP_ENABLED = "A mutually exclusive network already has termination enabled.";
static constexpr const char* ETH_PHY_REGISTER_CONTROL_NOT_AVAILABLE = "Ethernet PHY register control is not available for this device.";
static constexpr const char* DISK_NOT_SUPPORTED = "This device does not support accessing the specified disk.";
static constexpr const char* EOF_REACHED = "The requested length exceeds the available data from this disk.";
static constexpr const char* SETTINGS_DEFAULTS_USED = "The device settings could not be loaded, the default settings have been applied.";
static constexpr const char* ATOMIC_OPERATION_RETRIED = "An operation failed to be atomically completed, but will be retried.";
static constexpr const char* ATOMIC_OPERATION_COMPLETED_NONATOMICALLY = "An ideally-atomic operation was completed nonatomically.";
static constexpr const char* WIVI_STACK_REFRESH_FAILED = "The Wireless neoVI stack encountered a communication error.";
static constexpr const char* WIVI_UPLOAD_STACK_OVERFLOW = "The Wireless neoVI upload stack has encountered an overflow condition.";
static constexpr const char* A2B_MESSAGE_INCOMPLETE_FRAME = "At least one of the frames of the A2B message does not contain samples for each channel and stream."; 

// Transport Errors
static constexpr const char* FAILED_TO_READ = "A read operation failed.";
static constexpr const char* FAILED_TO_WRITE = "A write operation failed.";
static constexpr const char* DRIVER_FAILED_TO_OPEN = "The device driver encountered a low-level error while opening the device.";
static constexpr const char* DRIVER_FAILED_TO_CLOSE = "The device driver encountered a low-level error while closing the device.";
static constexpr const char* PACKET_CHECKSUM_ERROR = "There was a checksum error while decoding a packet. The packet was dropped.";
static constexpr const char* TRANSMIT_BUFFER_FULL = "The transmit buffer is full and the device is set to non-blocking.";
static constexpr const char* DEVICE_IN_USE = "The device is currently in use by another program.";
static constexpr const char* PCAP_COULD_NOT_START = "The PCAP driver could not be started. Ethernet devices will not be found.";
static constexpr const char* PCAP_COULD_NOT_FIND_DEVICES = "The PCAP driver failed to find devices. Ethernet devices will not be found.";
static constexpr const char* PACKET_DECODING = "There was an error decoding a packet from the device.";

static constexpr const char* SHARED_MEMORY_DATA_IS_NULL = "data() was called on invalid object.";
static constexpr const char* SHARED_MEMORY_FAILED_TO_CLOSE = "The server failed to close a shared memory location.";
static constexpr const char* SHARED_MEMORY_FAILED_TO_OPEN = "The server failed to open a shared memory location.";
static constexpr const char* SHARED_MEMORY_FAILED_TO_UNLINK = "The server failed to unlink a shared memory location.";
static constexpr const char* SHARED_MEMORY_FILE_TRUNCATE_ERROR = "The server failed to truncate shared memory file.";
static constexpr const char* SHARED_MEMORY_MAPPING_ERROR = "The server failed to map a shared memory file.";
static constexpr const char* SHARED_MEMORY_UNMAP_ERROR = "The server failed to unmap a shared memory file.";
static constexpr const char* SHARED_SEMAPHORE_FAILED_TO_CLOSE = "The server failed to close a shared semaphore.";
static constexpr const char* SHARED_SEMAPHORE_FAILED_TO_OPEN = "The server failed to open a shared semaphore.";
static constexpr const char* SHARED_SEMAPHORE_FAILED_TO_POST = "A post() call failed on a shared semaphore.";
static constexpr const char* SHARED_SEMAPHORE_FAILED_TO_UNLINK = "The server failed to unlink a shared semaphore.";
static constexpr const char* SHARED_SEMAPHORE_FAILED_TO_WAIT = "A wait() call failed on a shared semaphore.";
static constexpr const char* SHARED_SEMAPHORE_NOT_OPEN_FOR_POST = "post() was called on a shared semaphore that is not open.";
static constexpr const char* SHARED_SEMAPHORE_NOT_OPEN_FOR_WAIT = "wait() was called on a shared semaphore that is not open.";
static constexpr const char* SOCKET_FAILED_CONNECT = "A socket connection was attempted but failed.";
static constexpr const char* SOCKET_FAILED_OPEN = "A socket failed to open.";
static constexpr const char* SOCKET_FAILED_CLOSE = "A socket failed to close.";
static constexpr const char* SOCKET_FAILED_READ = "A socket read operation failed.";
static constexpr const char* SOCKET_FAILED_WRITE = "A socket write operation failed.";
static constexpr const char* ACCEPTOR_FAILED_BIND = "A socket acceptor failed to bind.";
static constexpr const char* ACCEPTOR_FAILED_LISTEN = "A socket acceptor failed to listen.";

static constexpr const char* TOO_MANY_EVENTS = "Too many events have occurred. The list has been truncated.";
static constexpr const char* UNKNOWN = "An unknown internal error occurred.";
static constexpr const char* NO_ERROR_FOUND = "No errors found.";
static constexpr const char* INVALID = "An invalid internal error occurred.";
const char* APIEvent::DescriptionForType(Type type) {
	switch(type) {
		// API Errors
		case Type::InvalidNeoDevice:
			return INVALID_NEODEVICE;
		case Type::RequiredParameterNull:
			return REQUIRED_PARAMETER_NULL;
		case Type::BufferInsufficient:
			return BUFFER_INSUFFICIENT;
		case Type::OutputTruncated:
			return OUTPUT_TRUNCATED;
		case Type::ParameterOutOfRange:
			return PARAMETER_OUT_OF_RANGE;
		case Type::DeviceCurrentlyOpen:
			return DEVICE_CURRENTLY_OPEN;
		case Type::DeviceCurrentlyClosed:
			return DEVICE_CURRENTLY_CLOSED;
		case Type::DeviceCurrentlyOnline:
			return DEVICE_CURRENTLY_ONLINE;
		case Type::DeviceCurrentlyOffline:
			return DEVICE_CURRENTLY_OFFLINE;
		case Type::DeviceCurrentlyPolling:
			return DEVICE_CURRENTLY_POLLING;
		case Type::DeviceNotCurrentlyPolling:
			return DEVICE_NOT_CURRENTLY_POLLING;
		case Type::UnsupportedTXNetwork:
			return UNSUPPORTED_TX_NETWORK;
		case Type::MessageMaxLengthExceeded:
			return MESSAGE_MAX_LENGTH_EXCEEDED;
		case Type::ValueNotYetPresent:
			return VALUE_NOT_YET_PRESENT;
		case Type::Timeout:
			return TIMEOUT;
		case Type::WiVINotSupported:
			return WIVI_NOT_SUPPORTED;

		// Device Errors
		case Type::PollingMessageOverflow:
			return POLLING_MESSAGE_OVERFLOW;
		case Type::NoSerialNumber:
			return NO_SERIAL_NUMBER;
		case Type::IncorrectSerialNumber:
			return INCORRECT_SERIAL_NUMBER;
		case Type::SettingsReadError:
			return SETTINGS_READ;
		case Type::SettingsVersionError:
			return SETTINGS_VERSION;
		case Type::SettingsLengthError:
			return SETTINGS_LENGTH;
		case Type::SettingsChecksumError:
			return SETTINGS_CHECKSUM;
		case Type::SettingsNotAvailable:
			return SETTINGS_NOT_AVAILABLE;
		case Type::SettingsReadOnly:
			return SETTINGS_READONLY;
		case Type::CANSettingsNotAvailable:
			return CAN_SETTINGS_NOT_AVAILABLE;
		case Type::CANFDSettingsNotAvailable:
			return CANFD_SETTINGS_NOT_AVAILABLE;
		case Type::LSFTCANSettingsNotAvailable:
			return LSFTCAN_SETTINGS_NOT_AVAILABLE;
		case Type::SWCANSettingsNotAvailable:
			return SWCAN_SETTINGS_NOT_AVAILABLE;
		case Type::BaudrateNotFound:
			return BAUDRATE_NOT_FOUND;
		case Type::UnexpectedNetworkType:
			return UNEXPECTED_NETWORK_TYPE;
		case Type::DeviceFirmwareOutOfDate:
			return DEVICE_FIRMWARE_OUT_OF_DATE;
		case Type::SettingsStructureMismatch:
			return SETTINGS_STRUCTURE_MISMATCH;
		case Type::SettingsStructureTruncated:
			return SETTINGS_STRUCTURE_TRUNCATED;
		case Type::NoDeviceResponse:
			return NO_DEVICE_RESPONSE;
		case Type::MessageFormattingError:
			return MESSAGE_FORMATTING;
		case Type::CANFDNotSupported:
			return CANFD_NOT_SUPPORTED;
		case Type::RTRNotSupported:
			return RTR_NOT_SUPPORTED;
		case Type::DeviceDisconnected:
			return DEVICE_DISCONNECTED;
		case Type::OnlineNotSupported:
			return ONLINE_NOT_SUPPORTED;
		case Type::TerminationNotSupportedDevice:
			return TERMINATION_NOT_SUPPORTED_DEVICE;
		case Type::TerminationNotSupportedNetwork:
			return TERMINATION_NOT_SUPPORTED_NETWORK;
		case Type::AnotherInTerminationGroupEnabled:
			return ANOTHER_IN_TERMINATION_GROUP_ENABLED;
		case Type::NoSerialNumberFW:
			return NO_SERIAL_NUMBER_FW;
		case Type::NoSerialNumber12V:
			return NO_SERIAL_NUMBER_12V;
		case Type::NoSerialNumberFW12V:
			return NO_SERIAL_NUMBER_FW_12V;
		case Type::EthPhyRegisterControlNotAvailable:
			return ETH_PHY_REGISTER_CONTROL_NOT_AVAILABLE;
		case Type::DiskNotSupported:
			return DISK_NOT_SUPPORTED;
		case Type::EOFReached:
			return EOF_REACHED;
		case Type::SettingsDefaultsUsed:
			return SETTINGS_DEFAULTS_USED;
		case Type::AtomicOperationRetried:
			return ATOMIC_OPERATION_RETRIED;
		case Type::AtomicOperationCompletedNonatomically:
			return ATOMIC_OPERATION_COMPLETED_NONATOMICALLY;
		case Type::WiVIStackRefreshFailed:
			return WIVI_STACK_REFRESH_FAILED;
		case Type::WiVIUploadStackOverflow:
			return WIVI_UPLOAD_STACK_OVERFLOW;
		case Type::A2BMessageIncompleteFrame:
			return A2B_MESSAGE_INCOMPLETE_FRAME;

		// Transport Errors
		case Type::FailedToRead:
			return FAILED_TO_READ;
		case Type::FailedToWrite:
			return FAILED_TO_WRITE;
		case Type::DriverFailedToOpen:
			return DRIVER_FAILED_TO_OPEN;
		case Type::DriverFailedToClose:
			return DRIVER_FAILED_TO_CLOSE;
		case Type::PacketChecksumError:
			return PACKET_CHECKSUM_ERROR;
		case Type::TransmitBufferFull:
			return TRANSMIT_BUFFER_FULL;
		case Type::DeviceInUse:
			return DEVICE_IN_USE;
		case Type::PCAPCouldNotStart:
			return PCAP_COULD_NOT_START;
		case Type::PCAPCouldNotFindDevices:
			return PCAP_COULD_NOT_FIND_DEVICES;
		case Type::PacketDecodingError:
			return PACKET_DECODING;

		// Device Sharing Server Events
		case Type::SharedMemoryDataIsNull:
			return SHARED_MEMORY_DATA_IS_NULL;
		case Type::SharedMemoryFailedToClose:
			return SHARED_MEMORY_FAILED_TO_CLOSE;
		case Type::SharedMemoryFailedToOpen:
			return SHARED_MEMORY_FAILED_TO_OPEN;
		case Type::SharedMemoryFailedToUnlink:
			return SHARED_MEMORY_FAILED_TO_UNLINK;
		case Type::SharedMemoryFileTruncateError:
			return SHARED_MEMORY_FILE_TRUNCATE_ERROR;
		case Type::SharedMemoryMappingError:
			return SHARED_MEMORY_MAPPING_ERROR;
		case Type::SharedMemoryUnmapError:
			return SHARED_MEMORY_UNMAP_ERROR;
		case Type::SharedSemaphoreFailedToClose:
			return SHARED_SEMAPHORE_FAILED_TO_CLOSE;
		case Type::SharedSemaphoreFailedToOpen:
			return SHARED_SEMAPHORE_FAILED_TO_OPEN;
		case Type::SharedSemaphoreFailedToPost:
			return SHARED_SEMAPHORE_FAILED_TO_POST;
		case Type::SharedSemaphoreFailedToUnlink:
			return SHARED_SEMAPHORE_FAILED_TO_UNLINK;
		case Type::SharedSemaphoreFailedToWait:
			return SHARED_SEMAPHORE_FAILED_TO_WAIT;
		case Type::SharedSemaphoreNotOpenForPost:
			return SHARED_SEMAPHORE_NOT_OPEN_FOR_POST;
		case Type::SharedSemaphoreNotOpenForWait:
			return SHARED_SEMAPHORE_NOT_OPEN_FOR_WAIT;
		case Type::SocketFailedToOpen:
			return SOCKET_FAILED_OPEN;
		case Type::SocketFailedToClose:
			return SOCKET_FAILED_CLOSE;
		case Type::SocketFailedToConnect:
			return SOCKET_FAILED_CONNECT;
		case Type::SocketFailedToRead:
			return SOCKET_FAILED_READ;
		case Type::SocketFailedToWrite:
			return SOCKET_FAILED_WRITE;
		case Type::SocketAcceptorFailedToBind:
			return ACCEPTOR_FAILED_BIND;
		case Type::SocketAcceptorFailedToListen:
			return ACCEPTOR_FAILED_LISTEN;

		// Other Errors
		case Type::TooManyEvents:
			return TOO_MANY_EVENTS;
		case Type::Unknown:
			return UNKNOWN;
		case Type::NoErrorFound:
			return NO_ERROR_FOUND;
		default:
			return INVALID;
	}
}

bool EventFilter::match(const APIEvent& event) const noexcept {
	if(type != APIEvent::Type::Any && type != event.getType())
		return false;
	
	if(matchOnDevicePtr && !event.isForDevice(device))
		return false;

	if(severity != APIEvent::Severity::Any && severity != event.getSeverity())
		return false;

	if(serial.length() != 0 && !event.isForDevice(serial))
		return false;

	return true;
}

neosocketeventfilter_t EventFilter::getNeoSocketEventFilter() const noexcept {
	neosocketeventfilter_t filterStruct;
	filterStruct.eventNumber = static_cast<decltype(filterStruct.eventNumber)>(type);
	filterStruct.severity = static_cast<decltype(filterStruct.severity)>(severity);
	if((serial.length() + 1) == sizeof(filterStruct.serial)) {
		std::memcpy(&filterStruct.serial[0], serial.c_str(), sizeof(filterStruct.serial));
	}
	return filterStruct;
}