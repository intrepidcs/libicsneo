#ifndef __LIVEDATA_H__
#define __LIVEDATA_H__
#ifdef __cplusplus

#include <cstdint>
#include <vector>
#include <memory>
#include <optional>
#include "icsneo/communication/command.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

typedef uint32_t LiveDataHandle;
static constexpr size_t MAX_LIVE_DATA_ENTRIES = 20;

enum class LiveDataCommand : uint32_t {
	STATUS = 0,
	SUBSCRIBE,
	UNSUBSCRIBE,
	RESPONSE,
	CLEAR_ALL,
	SET_VALUE,
};

enum class LiveDataStatus : uint32_t {
	SUCCESS = 0,
	ERR_UNKNOWN_COMMAND,
	ERR_HANDLE,
	ERR_DUPLICATE,
	ERR_FULL
};

enum LiveDataObjectType : uint16_t {
	MISC = 8,
	SNA = UINT16_MAX,
};

enum class LiveDataValueType : uint32_t {
	GPS_LATITUDE = 2,
	GPS_LONGITUDE,
	GPS_ALTITUDE,
	GPS_SPEED,
	GPS_VALID,
	GPS_ENABLE = 62,
	MANUAL_TRIGGER = 108,
	TIME_SINCE_MSG = 111,
	GPS_ACCURACY = 120,
	GPS_BEARING = 121,
	GPS_TIME = 122,
	GPS_TIME_VALID = 123,
	DAQ_ENABLE = 124,
	// The values below are unsigned 64-bit integers (LiveDataValueFormat::UNSIGNED_64): read them with LiveDataUtil::liveDataValueToUInt64
	// The times are nanoseconds since 1970 (UTC), the cursors are card sectors
	LOGGER_CREATE_TIME = 133,
	LOGGER_INSTALL_TIME = 134,
	LOGGER_LAST_SLEEP_TIME = 135,
	LOGGER_LAST_BOOT_TIME = 136,
	LOGGER_LAST_SLEEP_LOGGER_CURSOR = 137,
	LOGGER_LAST_SLEEP_ROOT_DIR_CURSOR = 138,
	// End of the raw 64-bit integer values, values added after this point are 32.32 fixed point unless stated otherwise
};

inline std::ostream& operator<<(std::ostream& os, const LiveDataCommand cmd) {
	switch (cmd) {
		case LiveDataCommand::STATUS: return os << "Status";
		case LiveDataCommand::SUBSCRIBE: return os << "Subscribe";
		case LiveDataCommand::UNSUBSCRIBE: return os << "Unsubscribe";
		case LiveDataCommand::RESPONSE: return os << "Response";
		case LiveDataCommand::CLEAR_ALL: return os << "Clear All";
		case LiveDataCommand::SET_VALUE: return os << "Set Value";
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const LiveDataStatus cmd) {
	switch (cmd) {
		case LiveDataStatus::SUCCESS: return os << "Success";
		case LiveDataStatus::ERR_UNKNOWN_COMMAND: return os << "Error: Unknown Command";
		case LiveDataStatus::ERR_HANDLE: return os << "Error: Handle";
		case LiveDataStatus::ERR_DUPLICATE: return os << "Error: Duplicate";
		case LiveDataStatus::ERR_FULL: return os << "Error: Argument limit reached";
	}
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const LiveDataValueType cmd) {
	switch (cmd) {
		case LiveDataValueType::GPS_LATITUDE: return os << "GPS Latitude";
		case LiveDataValueType::GPS_LONGITUDE: return os << "GPS Longitude";
		case LiveDataValueType::GPS_ALTITUDE: return os << "GPS Altitude";
		case LiveDataValueType::GPS_SPEED: return os << "GPS Speed";
		case LiveDataValueType::GPS_VALID: return os << "GPS Valid";
		case LiveDataValueType::GPS_ENABLE: return os << "GPS Enabled";
		case LiveDataValueType::GPS_ACCURACY: return os << "GPS Accuracy";
		case LiveDataValueType::GPS_BEARING: return os << "GPS Bearing";
		case LiveDataValueType::GPS_TIME: return os << "GPS Time";
		case LiveDataValueType::GPS_TIME_VALID: return os << "GPS Time Valid";
		case LiveDataValueType::DAQ_ENABLE: return os << "DAQ Enable";
		case LiveDataValueType::MANUAL_TRIGGER: return os << "Manual Trigger";
		case LiveDataValueType::TIME_SINCE_MSG: return os << "Time Since Msg";
		case LiveDataValueType::LOGGER_CREATE_TIME: return os << "Logger Create Time";
		case LiveDataValueType::LOGGER_INSTALL_TIME: return os << "Logger Install Time";
		case LiveDataValueType::LOGGER_LAST_SLEEP_TIME: return os << "Logger Last Sleep Time";
		case LiveDataValueType::LOGGER_LAST_BOOT_TIME: return os << "Logger Last Boot Time";
		case LiveDataValueType::LOGGER_LAST_SLEEP_LOGGER_CURSOR: return os << "Logger Last Sleep Logger Cursor";
		case LiveDataValueType::LOGGER_LAST_SLEEP_ROOT_DIR_CURSOR: return os << "Logger Last Sleep Root Dir Cursor";
	}
	return os;
}

#pragma pack(push,2)
struct LiveDataHeader {
	uint32_t version; // See LiveDataVersion
	uint32_t cmd;
	uint32_t handle;
};

struct LiveDataArgument {
	LiveDataObjectType objectType;
	uint32_t objectIndex;
	uint32_t signalIndex;
	LiveDataValueType valueType;
};

// How a LiveDataValue's 8 bytes are to be read. Firmware before the format existed sends 0 (the byte was reserved), so its
// values read as 32.32 fixed point, as they always were.
enum class LiveDataValueFormat : uint8_t {
	FIXED_32_32 = 0, // 32.32 fixed point (LiveDataUtil::liveDataValueToDouble)
	UNSIGNED_64 = 1, // an unsigned 64-bit integer (LiveDataUtil::liveDataValueToUInt64)
};

struct LiveDataValueHeader {
	uint16_t length = 0; // Number of bytes to follow header
	LiveDataValueFormat format = LiveDataValueFormat::FIXED_32_32; // a SET_VALUE is sent in its value's format
	uint8_t reserved = 0;
};

typedef struct
{
	LiveDataValueHeader header;
	int64_t value;
} LiveDataValue;

struct LiveDataValueResponse : public LiveDataHeader {
	uint32_t numArgs = 0;
	LiveDataValue values[1];
};

struct LiveDataStatusResponse : public LiveDataHeader {
	LiveDataCommand requestedCommand;
	LiveDataStatus status;
};

struct LiveDataSubscribe : public LiveDataHeader {
	uint32_t numArgs = 0;
	uint32_t freqMs;
	uint32_t expireMs = 0;
	LiveDataArgument args[1];
};

struct LiveDataSetValueEntry
{
	LiveDataArgument arg;
	LiveDataValue value;
};

struct LiveDataSetValue : public LiveDataHeader {
	uint32_t numSetValues = 0;
	LiveDataSetValueEntry values[1];
};

struct ExtResponseHeader {
	ExtendedCommand command;
	uint16_t length;
};
#pragma pack(pop)

namespace LiveDataUtil
{

LiveDataHandle getNewHandle();
// Use for LiveDataValueFormat::FIXED_32_32; std::nullopt for any other format
std::optional<double> liveDataValueToDouble(const LiveDataValue& val);
// Use for LiveDataValueFormat::FIXED_32_32; std::nullopt if the double does not fit in 32.32
std::optional<LiveDataValue> liveDataDoubleToValue(const double dFloat);
// Use for LiveDataValueFormat::UNSIGNED_64
LiveDataValue liveDataUInt64ToValue(uint64_t value);
// Use for LiveDataValueFormat::UNSIGNED_64; std::nullopt for any other format
std::optional<uint64_t> liveDataValueToUInt64(const LiveDataValue& val);
static constexpr uint32_t LiveDataVersion = 1;

} // namespace LiveDataUtil
} // namespace icsneo

#endif // _cplusplus
#endif // __LIVEDATA_H__
