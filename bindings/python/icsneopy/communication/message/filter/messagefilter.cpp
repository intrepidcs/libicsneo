#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/filter/messagefilter.h"

namespace icsneo {

void init_messagefilter(pybind11::module_& m) {
	pybind11::class_<MessageFilter, std::shared_ptr<MessageFilter>>(m, "MessageFilter")
		.def(pybind11::init())
		.def(pybind11::init<Network::NetID>());
}

} // namespace icsneo 

