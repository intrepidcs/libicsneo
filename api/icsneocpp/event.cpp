#include "icsneo/api/event.h"
#include "icsneo/device/device.h"
#include <sstream>

using namespace icsneo;

APIEvent::APIEvent(Type type, APIEvent::Severity severity, const Device* device) : eventStruct({}) {
	this->device = device;
	if(!device) {
		serial = device->getSerial();
		eventStruct.serial[serial.copy(eventStruct.serial, sizeof(eventStruct.serial))] = '\0';
	}

	init(type, severity);
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

// Device Errors
static constexpr const char* POLLING_MESSAGE_OVERFLOW = "Too many messages have been recieved for the polling message buffer, some have been lost!";
static constexpr const char* NO_SERIAL_NUMBER = "Communication could not be established with the device. Perhaps it is not powered with 12 volts?";
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

// Transport Errors
static constexpr const char* FAILED_TO_READ = "A read operation failed.";
static constexpr const char* FAILED_TO_WRITE = "A write operation failed.";
static constexpr const char* DRIVER_FAILED_TO_OPEN = "The device driver encountered a low-level error while opening the device.";
static constexpr const char* DRIVER_FAILED_TO_CLOSE = "The device driver encountered a low-level error while closing the device.";
static constexpr const char* PACKET_CHECKSUM_ERROR = "There was a checksum error while decoding a packet. The packet was dropped.";
static constexpr const char* TRANSMIT_BUFFER_FULL = "The transmit buffer is full and the device is set to non-blocking.";
static constexpr const char* PCAP_COULD_NOT_START = "The PCAP driver could not be started. Ethernet devices will not be found.";
static constexpr const char* PCAP_COULD_NOT_FIND_DEVICES = "The PCAP driver failed to find devices. Ethernet devices will not be found.";
static constexpr const char* PACKET_DECODING = "The packet could not be decoded.";

static constexpr const char* TOO_MANY_EVENTS = "Too many events have occurred. The list has been truncated.";
static constexpr const char* UNKNOWN = "An unknown internal error occurred.";
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
		case Type::PCAPCouldNotStart:
			return PCAP_COULD_NOT_START;
		case Type::PCAPCouldNotFindDevices:
			return PCAP_COULD_NOT_FIND_DEVICES;
		case Type::PacketDecodingError:
			return PACKET_DECODING;
		
		// Other Errors
		case Type::TooManyEvents:
			return TOO_MANY_EVENTS;
		case Type::Unknown:
			return UNKNOWN;
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