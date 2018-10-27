#include "icsneo/api/error.h"
#include "icsneo/device/device.h"
#include <sstream>

using namespace icsneo;

APIError::APIError(ErrorType error) : errorStruct({}) {
	init(error);
}

APIError::APIError(ErrorType error, const Device* forDevice) : errorStruct({}) {
	device = forDevice;
	serial = device->getSerial();
	errorStruct.serial[serial.copy(errorStruct.serial, sizeof(errorStruct.serial))] = '\0';

	init(error);
}

void APIError::init(ErrorType error) {
	timepoint = std::chrono::high_resolution_clock::now();
	errorStruct.description = DescriptionForType(error);
	errorStruct.errorNumber = (uint32_t)error;
	errorStruct.severity = (uint8_t)SeverityForType(error);
	errorStruct.timestamp = std::chrono::high_resolution_clock::to_time_t(timepoint);
}

std::string APIError::describe() const noexcept {
	std::stringstream ss;
	if(device)
		ss << *device; // Makes use of device.describe()
	else
		ss << "API";
	ss << " Error: ";
	ss << getDescription();
	return ss.str();
}

bool APIError::isForDevice(std::string serial) const noexcept {
	if(!device || serial.length() == 0)
		return false;
	return device->getSerial() == serial;
}

// API Errors
static constexpr const char* ERROR_INVALID_NEODEVICE = "The provided neodevice_t object was invalid.";
static constexpr const char* ERROR_REQUIRED_PARAMETER_NULL = "A required parameter was NULL.";
static constexpr const char* ERROR_BUFFER_INSUFFICIENT = "The provided buffer was insufficient. No data was written.";
static constexpr const char* ERROR_OUTPUT_TRUNCATED = "The output was too large for the provided buffer and has been truncated.";
static constexpr const char* ERROR_PARAMETER_OUT_OF_RANGE = "A parameter was out of range.";

// Device Errors
static constexpr const char* ERROR_POLLING_MESSAGE_OVERFLOW = "Too many messages have been recieved for the polling message buffer, some have been lost!";
static constexpr const char* ERROR_NO_SERIAL_NUMBER = "Communication could not be established with the device. Perhaps it is not powered with 12 volts?";
static constexpr const char* ERROR_INCORRECT_SERIAL_NUMBER = "The device did not return the expected serial number!";
static constexpr const char* ERROR_SETTINGS_READ = "The device settings could not be read.";
static constexpr const char* ERROR_SETTINGS_VERSION = "The settings version is incorrect, please update your firmware with neoVI Explorer.";
static constexpr const char* ERROR_SETTINGS_LENGTH = "The settings length is incorrect, please update your firmware with neoVI Explorer.";
static constexpr const char* ERROR_SETTINGS_CHECKSUM = "The settings checksum is incorrect, attempting to set defaults may remedy this issue.";
static constexpr const char* ERROR_SETTINGS_NOT_AVAILABLE = "Settings are not available for this device.";

static constexpr const char* ERROR_TOO_MANY_ERRORS = "Too many errors have occurred. The list has been truncated.";
static constexpr const char* ERROR_UNKNOWN = "An unknown internal error occurred.";
static constexpr const char* ERROR_INVALID = "An invalid internal error occurred.";
const char* APIError::DescriptionForType(ErrorType type) {
	switch(type) {
		// API Errors
		case InvalidNeoDevice:
			return ERROR_INVALID_NEODEVICE;
		case RequiredParameterNull:
			return ERROR_REQUIRED_PARAMETER_NULL;
		case BufferInsufficient:
			return ERROR_BUFFER_INSUFFICIENT;
		case OutputTruncated:
			return ERROR_OUTPUT_TRUNCATED;
		case ParameterOutOfRange:
			return ERROR_PARAMETER_OUT_OF_RANGE;
		
		// Device Errors
		case PollingMessageOverflow:
			return ERROR_POLLING_MESSAGE_OVERFLOW;
		case NoSerialNumber:
			return ERROR_NO_SERIAL_NUMBER;
		case IncorrectSerialNumber:
			return ERROR_INCORRECT_SERIAL_NUMBER;
		case SettingsReadError:
			return ERROR_SETTINGS_READ;
		case SettingsVersionError:
			return ERROR_SETTINGS_VERSION;
		case SettingsLengthError:
			return ERROR_SETTINGS_LENGTH;
		case SettingsChecksumError:
			return ERROR_SETTINGS_CHECKSUM;
		case SettingsNotAvailable:
			return ERROR_SETTINGS_NOT_AVAILABLE;
		
		// Other Errors
		case TooManyErrors:
			return ERROR_TOO_MANY_ERRORS;
		case Unknown:
			return ERROR_UNKNOWN;
		default:
			return ERROR_INVALID;
	}
}

APIError::Severity APIError::SeverityForType(ErrorType type) {
	switch(type) {
		// API Warnings
		case OutputTruncated:
		// Device Warnings
		case PollingMessageOverflow:
			return Severity::Warning;

		// API Errors
		case InvalidNeoDevice:
		case RequiredParameterNull:
		case BufferInsufficient:
		case ParameterOutOfRange:
		// Device Errors
		case NoSerialNumber:
		case IncorrectSerialNumber:
		case SettingsReadError:
		case SettingsVersionError:
		case SettingsLengthError:
		case SettingsChecksumError:
		case SettingsNotAvailable:
		// Other Errors
		case TooManyErrors:
		case Unknown:
		default:
			return Severity::Error;
	}
}

bool ErrorFilter::match(const APIError& error) const noexcept {
	if(type != APIError::Any && type != error.getType())
		return false;
	
	if(matchOnDevicePtr && !error.isForDevice(device))
		return false;

	if(severity != APIError::Severity::Any && severity != error.getSeverity())
		return false;

	if(serial.length() != 0 && !error.isForDevice(serial))
		return false;

	return true;
}