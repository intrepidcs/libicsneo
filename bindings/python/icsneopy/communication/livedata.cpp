#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/native_enum.h>

#include "icsneo/communication/livedata.h"

namespace icsneo {

void init_livedata(pybind11::module_& m) {
	// LiveDataValue struct
	pybind11::classh<LiveDataValue>(m, "LiveDataValue")
		.def(pybind11::init<>())
		.def_readwrite("value", &LiveDataValue::value);

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
		.finalize();

	// LiveDataUtil namespace functions
	m.def("get_new_handle", &LiveDataUtil::getNewHandle,
		"Generate a new unique LiveData handle");

	m.def("livedata_value_to_double", &LiveDataUtil::liveDataValueToDouble,
		pybind11::arg("val"),
		"Convert LiveDataValue to double (32.32 fixed-point to floating-point)");

	m.def("livedata_double_to_value", &LiveDataUtil::liveDataDoubleToValue,
		pybind11::arg("d"),
		"Convert double to LiveDataValue (32.32 fixed-point format). Returns LiveDataValue or None on failure.");
}

} // namespace icsneo
