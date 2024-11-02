#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/api/eventcallback.h"

namespace icsneo {

void init_eventcallback(pybind11::module_& m) {
	pybind11::class_<EventCallback>(m, "EventCallback")
		.def(pybind11::init<EventCallback::fn_eventCallback, EventFilter>())
		.def(pybind11::init<EventCallback::fn_eventCallback>());
}

} // namespace icsneo 

