#ifndef __ICSNEO_API_ERROR_H_
#define __ICSNEO_API_ERROR_H_

#include <stdint.h>
#include <time.h>

typedef struct {
	const char* description;
	time_t timestamp;
	uint32_t errorNumber;
	uint8_t severity;
	char serial[7];
	uint8_t reserved[16];
} neoerror_t;

#ifdef __cplusplus

#include <vector>
#include <chrono>
#include <string>
#include <ostream>

namespace icsneo {

class Device;

class APIError {
public:
	typedef std::chrono::system_clock ErrorClock;
	typedef std::chrono::time_point<ErrorClock> ErrorTimePoint;

	enum ErrorType : uint32_t {
		Any = 0, // Used for filtering, should not appear in data

		// API Errors
		InvalidNeoDevice = 0x1000,
		RequiredParameterNull = 0x1001,
		BufferInsufficient = 0x1002,
		OutputTruncated = 0x1003,
		ParameterOutOfRange = 0x1004,
		DeviceCurrentlyOpen = 0x1005,
		DeviceCurrentlyClosed = 0x1006,

		// Device Errors
		PollingMessageOverflow = 0x2000,
		NoSerialNumber = 0x2001,
		IncorrectSerialNumber = 0x2002,
		SettingsReadError = 0x2003,
		SettingsVersionError = 0x2004,
		SettingsLengthError = 0x2005,
		SettingsChecksumError = 0x2006,
		SettingsNotAvailable = 0x2007,
		SettingsReadOnly = 0x2008,
		CANSettingsNotAvailable = 0x2009,
		CANFDSettingsNotAvailable = 0x2010,
		LSFTCANSettingsNotAvailable = 0x2011,
		SWCANSettingsNotAvailable = 0x2012,
		BaudrateNotFound = 0x2013,
		BadNetworkType = 0x2014,
		DeviceFirmwareOutOfDate = 0x2015,
		SettingsStructureMismatch = 0x2016,
		SettingsStructureTruncated = 0x2017,
		NoDeviceResponse = 0x2018,

		// Transport Errors
		FailedToRead = 0x3000,
		FailedToWrite = 0x3001,
		DriverFailedToOpen = 0x3002,
		DriverFailedToClose = 0x3003,
		PacketChecksumError = 0x3004,
		TransmitBufferFull = 0x3005,
		PCAPCouldNotStart = 0x3102,
		PCAPCouldNotFindDevices = 0x3103,

		TooManyErrors = 0xFFFFFFFE,
		Unknown = 0xFFFFFFFF
	};
	enum class Severity : uint8_t {
		Any = 0, // Used for filtering, should not appear in data
		Info = 0x10,
		Warning = 0x20,
		Error = 0x30
	};

	APIError() : errorStruct({}), device(nullptr) {}
	APIError(ErrorType error);
	APIError(ErrorType error, const Device* device);

	const neoerror_t* getNeoError() const noexcept { return &errorStruct; }
	ErrorType getType() const noexcept { return ErrorType(errorStruct.errorNumber); }
	Severity getSeverity() const noexcept { return Severity(errorStruct.severity); }
	std::string getDescription() const noexcept { return std::string(errorStruct.description); }
	const Device* getDevice() const noexcept { return device; } // Will return nullptr if this is an API-wide error
	ErrorTimePoint getTimestamp() const noexcept { return timepoint; }

	bool isForDevice(const Device* forDevice) const noexcept { return forDevice == device; }
	bool isForDevice(std::string serial) const noexcept;
	
	// As opposed to getDescription, this will also add text such as "neoVI FIRE 2 CY2468 Error: " to fully describe the problem
	std::string describe() const noexcept;
	friend std::ostream& operator<<(std::ostream& os, const APIError& error) {
		os << error.describe();
		return os;
	}

	static const char* DescriptionForType(ErrorType type);
	static Severity SeverityForType(ErrorType type);

private:
	neoerror_t errorStruct;
	std::string serial;
	ErrorTimePoint timepoint;
	const Device* device;

	void init(ErrorType error);
};

class ErrorFilter {
public:
	ErrorFilter() {} // Empty filter matches anything
	ErrorFilter(APIError::ErrorType error) : type(error) {}
	ErrorFilter(APIError::Severity severity) : severity(severity) {}
	ErrorFilter(const Device* device, APIError::ErrorType error = APIError::Any) : type(error), matchOnDevicePtr(true), device(device) {}
	ErrorFilter(const Device* device, APIError::Severity severity) : severity(severity), matchOnDevicePtr(true), device(device) {}
	ErrorFilter(std::string serial, APIError::ErrorType error = APIError::Any) : type(error), serial(serial) {}
	ErrorFilter(std::string serial, APIError::Severity severity) : severity(severity), serial(serial) {}

	bool match(const APIError& error) const noexcept;

	APIError::Severity severity = APIError::Severity::Any;
	APIError::ErrorType type = APIError::Any;
	bool matchOnDevicePtr = false;
	const Device* device = nullptr; // nullptr will match on "no device, generic API error"
	std::string serial; // Empty serial will match any, including no device. Not affected by matchOnDevicePtr
};

}

#endif // __cplusplus

#endif