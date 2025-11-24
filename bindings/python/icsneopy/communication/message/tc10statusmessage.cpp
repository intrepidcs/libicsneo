#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/tc10statusmessage.h"

namespace icsneo {

void init_tc10statusmessage(pybind11::module_& m) {
	pybind11::enum_<TC10WakeStatus>(m, "TC10WakeStatus")
		.value("NoWakeReceived", TC10WakeStatus::NoWakeReceived)
		.value("WakeReceived", TC10WakeStatus::WakeReceived);

	pybind11::enum_<TC10SleepStatus>(m, "TC10SleepStatus")
		.value("NoSleepReceived", TC10SleepStatus::NoSleepReceived)
		.value("SleepReceived", TC10SleepStatus::SleepReceived)
		.value("SleepFailed", TC10SleepStatus::SleepFailed)
		.value("SleepAborted", TC10SleepStatus::SleepAborted);

	pybind11::classh<TC10StatusMessage, Message>(m, "TC10StatusMessage")
		.def_readonly("wakeStatus", &TC10StatusMessage::wakeStatus)
		.def_readonly("sleepStatus", &TC10StatusMessage::sleepStatus);
}

} // namespace icsneo
