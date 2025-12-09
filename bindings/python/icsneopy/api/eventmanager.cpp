#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/api/eventmanager.h"

namespace icsneo {

void init_eventmanager(pybind11::module_& m) {
	pybind11::classh<EventManager>(m, "EventManager")
		.def_static("get_instance", &EventManager::GetInstance, pybind11::return_value_policy::reference)
		.def("add_event_callback", &EventManager::addEventCallback)
		.def("remove_event_callback", &EventManager::removeEventCallback)
		.def("get_last_error", &EventManager::getLastError);
}

} // namespace icsneo 

