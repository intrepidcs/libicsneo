#ifndef __LIVEDATA_H__
#define __LIVEDATA_H__
#ifdef __cplusplus

#include <cstdint>
#include <vector>
#include <memory>
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

struct LiveDataValueHeader {
	uint16_t length; // Number of bytes to follow header
	uint8_t reserved[2];
};

typedef struct
{
	LiveDataValueHeader header;
	int64_t value;
} LiveDataValue;

struct LiveDataValueResponse : public LiveDataHeader {
	uint32_t numArgs;
	LiveDataValue values[1];
};

struct LiveDataStatusResponse : public LiveDataHeader {
	LiveDataCommand requestedCommand;
	LiveDataStatus status;
};

struct LiveDataSubscribe : public LiveDataHeader {
	uint32_t numArgs;
	uint32_t freqMs;
	uint32_t expireMs;
	LiveDataArgument args[1];
};

struct LiveDataSetValueEntry
{
	LiveDataArgument arg;
	LiveDataValue value;
};

struct LiveDataSetValue : public LiveDataHeader {
	uint32_t numSetValues;
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
double liveDataValueToDouble(const LiveDataValue& val);
bool liveDataDoubleToValue(const double& dFloat, LiveDataValue& value);
static constexpr uint32_t LiveDataVersion = 1;

} // namespace LiveDataUtil
} // namespace icsneo

#endif // _cplusplus
#endif // __LIVEDATA_H__
