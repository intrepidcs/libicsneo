#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/native_enum.h>

#include "icsneo/communication/livedata.h"

namespace icsneo {

void init_livedata(pybind11::module_& m) {
	// LiveDataValueFormat enum
	pybind11::native_enum<LiveDataValueFormat>(m, "LiveDataValueFormat", "enum.IntEnum")
		.value("FIXED_32_32", LiveDataValueFormat::FIXED_32_32)
		.value("UNSIGNED_64", LiveDataValueFormat::UNSIGNED_64)
		.finalize();

	pybind11::classh<LiveDataValueHeader>(m, "LiveDataValueHeader")
		.def(pybind11::init<>())
		.def_readwrite("length", &LiveDataValueHeader::length)
		.def_readwrite("format", &LiveDataValueHeader::format)
		.def_readwrite("reserved", &LiveDataValueHeader::reserved);

	// LiveDataValue struct
	pybind11::classh<LiveDataValue>(m, "LiveDataValue")
		.def(pybind11::init<>())
		.def_readwrite("value", &LiveDataValue::value)
		.def_readwrite("header", &LiveDataValue::header);

	// LiveDataArgument struct
	pybind11::classh<LiveDataArgument>(m, "LiveDataArgument")
		.def(pybind11::init<>())
		.def_readwrite("object_type", &LiveDataArgument::objectType)
		.def_readwrite("object_index", &LiveDataArgument::objectIndex)
		.def_readwrite("signal_index", &LiveDataArgument::signalIndex)
		.def_readwrite("value_type", &LiveDataArgument::valueType);

	// LiveDataCommand enum
	pybind11::native_enum<LiveDataCommand>(m, "LiveDataCommand", "enum.IntEnum")
		.value("STATUS", LiveDataCommand::STATUS)
		.value("SUBSCRIBE", LiveDataCommand::SUBSCRIBE)
		.value("UNSUBSCRIBE", LiveDataCommand::UNSUBSCRIBE)
		.value("RESPONSE", LiveDataCommand::RESPONSE)
		.value("CLEAR_ALL", LiveDataCommand::CLEAR_ALL)
		.value("SET_VALUE", LiveDataCommand::SET_VALUE)
		.finalize();

	// LiveDataStatus enum
	pybind11::native_enum<LiveDataStatus>(m, "LiveDataStatus", "enum.IntEnum")
		.value("SUCCESS", LiveDataStatus::SUCCESS)
		.value("ERR_UNKNOWN_COMMAND", LiveDataStatus::ERR_UNKNOWN_COMMAND)
		.value("ERR_HANDLE", LiveDataStatus::ERR_HANDLE)
		.value("ERR_DUPLICATE", LiveDataStatus::ERR_DUPLICATE)
		.value("ERR_FULL", LiveDataStatus::ERR_FULL)
		.finalize();

	// LiveDataObjectType enum
	pybind11::enum_<LiveDataObjectType>(m, "LiveDataObjectType")
		.value("MISC", LiveDataObjectType::MISC)
		.value("SNA", LiveDataObjectType::SNA)
		.export_values();

	// LiveDataValueType enum
	pybind11::native_enum<LiveDataValueType>(m, "LiveDataValueType", "enum.IntEnum")
		.value("GPS_LATITUDE", LiveDataValueType::GPS_LATITUDE)
		.value("GPS_LONGITUDE", LiveDataValueType::GPS_LONGITUDE)
		.value("GPS_ALTITUDE", LiveDataValueType::GPS_ALTITUDE)
		.value("GPS_SPEED", LiveDataValueType::GPS_SPEED)
		.value("GPS_VALID", LiveDataValueType::GPS_VALID)
		.value("GPS_ENABLE", LiveDataValueType::GPS_ENABLE)
		.value("MANUAL_TRIGGER", LiveDataValueType::MANUAL_TRIGGER)
		.value("TIME_SINCE_MSG", LiveDataValueType::TIME_SINCE_MSG)
		.value("GPS_ACCURACY", LiveDataValueType::GPS_ACCURACY)
		.value("GPS_BEARING", LiveDataValueType::GPS_BEARING)
		.value("GPS_TIME", LiveDataValueType::GPS_TIME)
		.value("GPS_TIME_VALID", LiveDataValueType::GPS_TIME_VALID)
		.value("DAQ_ENABLE", LiveDataValueType::DAQ_ENABLE)
		.value("LOGGER_CREATE_TIME", LiveDataValueType::LOGGER_CREATE_TIME)
		.value("LOGGER_INSTALL_TIME", LiveDataValueType::LOGGER_INSTALL_TIME)
		.value("LOGGER_LAST_SLEEP_TIME", LiveDataValueType::LOGGER_LAST_SLEEP_TIME)
		.value("LOGGER_LAST_BOOT_TIME", LiveDataValueType::LOGGER_LAST_BOOT_TIME)
		.value("LOGGER_LAST_SLEEP_LOGGER_CURSOR", LiveDataValueType::LOGGER_LAST_SLEEP_LOGGER_CURSOR)
		.value("LOGGER_LAST_SLEEP_ROOT_DIR_CURSOR", LiveDataValueType::LOGGER_LAST_SLEEP_ROOT_DIR_CURSOR)
		.finalize();

	// LiveDataUtil namespace functions
	m.def("get_new_handle", &LiveDataUtil::getNewHandle,
		"Generate a new unique LiveData handle");

	m.def("livedata_value_to_double", &LiveDataUtil::liveDataValueToDouble,
		pybind11::arg("val"),
		"Convert LiveDataValue to double: Use with LiveDataValueFormat.FIXED_32_32. Returns None for any other format.");

	m.def("livedata_value_to_uint64", &LiveDataUtil::liveDataValueToUInt64,
		pybind11::arg("val"),
		"Convert LiveDataValue to uint64: Use with LiveDataValueFormat.UNSIGNED_64. Returns None for any other format.");

	m.def("livedata_double_to_value", &LiveDataUtil::liveDataDoubleToValue,
		pybind11::arg("d"),
		"Convert double to LiveDataValue (32.32 fixed-point format). Returns LiveDataValue or None on failure.");

	m.def("livedata_uint64_to_value", &LiveDataUtil::liveDataUInt64ToValue,
		pybind11::arg("value"),
		"Convert uint64 to LiveDataValue (LiveDataValueFormat.UNSIGNED_64). Returns LiveDataValue.");
}

} // namespace icsneo
