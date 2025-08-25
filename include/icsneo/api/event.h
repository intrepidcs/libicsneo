#ifndef __ICSNEO_API_EVENT_H_
#define __ICSNEO_API_EVENT_H_

#include <stdint.h>
#include <time.h>

typedef struct {
	const char* description;
	time_t timestamp;
	uint32_t eventNumber;
	uint8_t severity;
	char serial[7];
	uint8_t reserved[16];
} neoevent_t;

#ifdef __cplusplus

#include <vector>
#include <chrono>
#include <string>
#include <ostream>

namespace icsneo {

class Device;

class APIEvent {
public:
	typedef std::chrono::system_clock EventClock;
	typedef std::chrono::time_point<EventClock> EventTimePoint;

	enum class Type : uint32_t {
		Any = 0, // Used for filtering, should not appear in data

		// API Events
		InvalidNeoDevice = 0x1000, // api
		RequiredParameterNull = 0x1001,
		BufferInsufficient = 0x1002,
		OutputTruncated = 0x1003, // just a warning
		ParameterOutOfRange = 0x1004,
		DeviceCurrentlyOpen = 0x1005,
		DeviceCurrentlyClosed = 0x1006,
		DeviceCurrentlyOnline = 0x1007,
		DeviceCurrentlyOffline = 0x1008,
		DeviceCurrentlyPolling = 0x1009,
		DeviceNotCurrentlyPolling = 0x1010,
		UnsupportedTXNetwork = 0x1011,
		MessageMaxLengthExceeded = 0x1012,
		ValueNotYetPresent = 0x1013,
		Timeout = 0x1014,
		WiVINotSupported = 0x1015,
		RestrictedEntryFlag = 0x1016,
		NotSupported = 0x1017,
		FixedPointOverflow = 0x1018,
		FixedPointPrecision = 0x1019,
		SyscallError = 0x1020, // check errno/GetLastError() for details

		// Device Events
		PollingMessageOverflow = 0x2000,
		NoSerialNumber = 0x2001, // api
		IncorrectSerialNumber = 0x2002, // api
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
		UnexpectedNetworkType = 0x2014,
		DeviceFirmwareOutOfDate = 0x2015,
		SettingsStructureMismatch = 0x2016,
		SettingsStructureTruncated = 0x2017,
		NoDeviceResponse = 0x2018,
		MessageFormattingError = 0x2019,
		CANFDNotSupported = 0x2020,
		RTRNotSupported = 0x2021,
		DeviceDisconnected = 0x2022,
		OnlineNotSupported = 0x2023,
		TerminationNotSupportedDevice = 0x2024,
		TerminationNotSupportedNetwork = 0x2025,
		AnotherInTerminationGroupEnabled = 0x2026,
		NoSerialNumberFW = 0x2027, // A firmware update was already attempted
		NoSerialNumber12V = 0x2028, // The device must be powered with 12V for communication to be established
		NoSerialNumberFW12V = 0x2029, // The device must be powered with 12V for communication to be established, a firmware update was already attempted
		EthPhyRegisterControlNotAvailable = 0x2030, //The device doesn't support Ethernet PHY MDIO access
		DiskNotSupported = 0x2031,
		EOFReached = 0x2032,
		SettingsDefaultsUsed = 0x2033,
		AtomicOperationRetried = 0x2034,
		AtomicOperationCompletedNonatomically = 0x2035,
		WiVIStackRefreshFailed = 0x2036,
		WiVIUploadStackOverflow = 0x2037,
		I2CMessageExceedsMaxLength = 0x2038,
		A2BMessageIncompleteFrame = 0x2039,
		CoreminiUploadVersionMismatch = 0x2040,
		DiskNotConnected = 0x2041,
		UnexpectedResponse = 0x2042,
		LiveDataInvalidHandle = 0x2043,
		LiveDataInvalidCommand = 0x2044,
		LiveDataInvalidArgument = 0x2045,
		LiveDataVersionMismatch = 0x2046,
		LiveDataNoDeviceResponse = 0x2047,
		LiveDataMaxSignalsReached = 0x2048,
		LiveDataCommandFailed = 0x2049,
		LiveDataEncoderError = 0x2050,
		LiveDataDecoderError = 0x2051,
		LiveDataNotSupported = 0x2052,
		LINSettingsNotAvailable = 0x2053,
		ModeNotFound = 0x2054,
		AppErrorParsingFailed = 0x2055,
		GPTPNotSupported = 0x2056,
		SettingNotAvaiableDevice = 0x2057,
		DiskFormatNotSupported = 0x2058,
		DiskFormatInvalidCount = 0x2059,

		// Transport Events
		FailedToRead = 0x3000,
		FailedToWrite = 0x3001,
		DriverFailedToOpen = 0x3002,
		DriverFailedToClose = 0x3003,
		PacketChecksumError = 0x3004,
		TransmitBufferFull = 0x3005,
		DeviceInUse = 0x3006,
		PCAPCouldNotStart = 0x3102,
		PCAPCouldNotFindDevices = 0x3103,
		PacketDecodingError = 0x3104,
		SocketFailedToOpen = 0x3105,
		FailedToBind = 0x3106,
		ErrorSettingSocketOption = 0x3107,
		GetIfAddrsError = 0x3108,
		SendToError = 0x3109,
		MDIOMessageExceedsMaxLength = 0x3110,

		// VSA
		VSABufferCorrupted = 0x5000,
		VSATimestampNotFound = VSABufferCorrupted + 1,
		VSABufferFormatError = VSABufferCorrupted + 2,
		VSAMaxReadAttemptsReached = VSABufferCorrupted + 3,
		VSAByteParseFailure = VSABufferCorrupted + 4,
		VSAExtendedMessageError = VSABufferCorrupted + 5,
		VSAOtherError = VSABufferCorrupted + 6,

		// Servd
		ServdBindError = 0x6000,
		ServdNonblockError = ServdBindError + 1,
		ServdTransceiveError = ServdBindError + 2,
		ServdOutdatedError = ServdBindError + 3,
		ServdInvalidResponseError = ServdBindError + 4,
		ServdLockError = ServdBindError + 5,
		ServdSendError = ServdBindError + 6,
		ServdRecvError = ServdBindError + 7,
		ServdPollError = ServdBindError + 8,
		ServdNoDataError = ServdBindError + 9,
		ServdJoinMulticastError = ServdBindError + 10,

		// DXX
		DXXErrorSys = 0x6100,
		DXXErrorInt = 0x6101,
		DXXErrorOverflow = 0x6102,
		DXXErrorIO = 0x6103,
		DXXErrorArg = 0x6104,

		NoErrorFound = 0xFFFFFFFD,
		TooManyEvents = 0xFFFFFFFE,
		Unknown = 0xFFFFFFFF
	};
	enum class Severity : uint8_t {
		Any = 0, // Used for filtering, should not appear in data
		EventInfo = 0x10,
		EventWarning = 0x20,
		Error = 0x30
	};

	APIEvent() : eventStruct({}), serial(), timepoint(), device(nullptr) {}
	APIEvent(APIEvent::Type event, APIEvent::Severity severity, const Device* device = nullptr);

	const neoevent_t* getNeoEvent() const noexcept { return &eventStruct; }
	Type getType() const noexcept { return Type(eventStruct.eventNumber); }
	Severity getSeverity() const noexcept { return Severity(eventStruct.severity); }
	std::string getDescription() const noexcept { return std::string(eventStruct.description); }
	const Device* getDevice() const noexcept { return device; } // Will return nullptr if this is an API-wide event
	EventTimePoint getTimestamp() const noexcept { return timepoint; }

	void downgradeFromError() noexcept;

	bool isForDevice(const Device* forDevice) const noexcept { return forDevice == device; }
	bool isForDevice(std::string serial) const noexcept;

	// As opposed to getDescription, this will also add text such as "neoVI FIRE 2 CY2468 Error: " to fully describe the problem
	std::string describe() const noexcept;
	friend std::ostream& operator<<(std::ostream& os, const APIEvent& event) {
		os << event.describe();
		return os;
	}

	static const char* DescriptionForType(Type type);

private:
	neoevent_t eventStruct;
	std::string serial;
	EventTimePoint timepoint;
	const Device* device;

	void init(APIEvent::Type event, APIEvent::Severity);
};

class EventFilter {
public:
	EventFilter() {} // Empty filter matches anything
	EventFilter(APIEvent::Type type, APIEvent::Severity severity = APIEvent::Severity::Any) : type(type), severity(severity) {}
	EventFilter(APIEvent::Severity severity) : severity(severity) {}
	EventFilter(const Device* device, APIEvent::Type type = APIEvent::Type::Any, APIEvent::Severity severity = APIEvent::Severity::Any) : type(type), severity(severity), matchOnDevicePtr(true), device(device) {}
	EventFilter(const Device* device, APIEvent::Severity severity) : severity(severity), matchOnDevicePtr(true), device(device) {}
	EventFilter(std::string serial, APIEvent::Type type = APIEvent::Type::Any, APIEvent::Severity severity = APIEvent::Severity::Any) : type(type), severity(severity), serial(serial) {}
	EventFilter(std::string serial, APIEvent::Severity severity) : severity(severity), serial(serial) {}

	bool match(const APIEvent& event) const noexcept;

	APIEvent::Type type = APIEvent::Type::Any;
	APIEvent::Severity severity = APIEvent::Severity::Any;
	bool matchOnDevicePtr = false;
	const Device* device = nullptr; // nullptr will match on "no device, generic API event"
	std::string serial; // Empty serial will match any, including no device. Not affected by matchOnDevicePtr
};

}

#endif // __cplusplus

#endif
