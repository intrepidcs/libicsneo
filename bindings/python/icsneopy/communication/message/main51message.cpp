#include <pybind11/pybind11.h>

#include "icsneo/communication/message/main51message.h"

namespace icsneo {

void init_main51message(pybind11::module_& m) {
	pybind11::classh<Main51Message, RawMessage>(m, "Main51Message")
		.def(pybind11::init<>())
		.def_readonly("command", &Main51Message::command);
}

} // namespace icsneo
