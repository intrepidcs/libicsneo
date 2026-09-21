#include <pybind11/pybind11.h>

#include "icsneo/communication/message/filter/main51messagefilter.h"

namespace icsneo {

void init_main51messagefilter(pybind11::module_& m) {
	pybind11::classh<Main51MessageFilter, MessageFilter>(m, "Main51MessageFilter")
		.def(pybind11::init<>())
		.def(pybind11::init<Command>());
}

} // namespace icsneo
