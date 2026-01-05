#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include "icsneo/communication/message/livedatamessage.h"

namespace icsneo {

void init_livedatamessage(pybind11::module_& m) {
	// LiveDataMessage base class
	pybind11::classh<LiveDataMessage, RawMessage>(m, "LiveDataMessage")
		.def(pybind11::init<>())
		.def_readwrite("handle", &LiveDataMessage::handle)
		.def_readwrite("cmd", &LiveDataMessage::cmd);

	// LiveDataCommandMessage (for subscribe/unsubscribe)
	pybind11::classh<LiveDataCommandMessage, LiveDataMessage>(m, "LiveDataCommandMessage")
		.def(pybind11::init<>())
		.def_readwrite("update_period", &LiveDataCommandMessage::updatePeriod)
		.def_readwrite("expiration_time", &LiveDataCommandMessage::expirationTime)
		.def_readwrite("args", &LiveDataCommandMessage::args)
		.def("append_signal_arg", &LiveDataCommandMessage::appendSignalArg,
			pybind11::arg("value_type"),
			"Append a signal argument to the command message");

	// LiveDataValueMessage (received values)
	pybind11::classh<LiveDataValueMessage, LiveDataMessage>(m, "LiveDataValueMessage")
		.def(pybind11::init<>())
		.def_readwrite("num_args", &LiveDataValueMessage::numArgs)
		.def_readwrite("values", &LiveDataValueMessage::values);

	// LiveDataStatusMessage (status responses)
	pybind11::classh<LiveDataStatusMessage, LiveDataMessage>(m, "LiveDataStatusMessage")
		.def(pybind11::init<>())
		.def_readwrite("requested_command", &LiveDataStatusMessage::requestedCommand)
		.def_readwrite("status", &LiveDataStatusMessage::status);

	// LiveDataSetValueMessage (for setting values)
	pybind11::classh<LiveDataSetValueMessage, LiveDataMessage>(m, "LiveDataSetValueMessage")
		.def(pybind11::init<>())
		.def_readwrite("args", &LiveDataSetValueMessage::args)
		.def_readwrite("values", &LiveDataSetValueMessage::values)
		.def("append_set_value", &LiveDataSetValueMessage::appendSetValue,
			pybind11::arg("value_type"),
			pybind11::arg("value"),
			"Append a value to set in the message");
}

} // namespace icsneo
