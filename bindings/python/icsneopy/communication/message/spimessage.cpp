#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/spimessage.h"

namespace icsneo {

void init_spimessage(pybind11::module_& m) {
	pybind11::classh<SPIMessage, Frame> spiMessage(m, "SPIMessage");
	pybind11::enum_<SPIMessage::Direction>(spiMessage, "Direction")
		.value("Write", SPIMessage::Direction::Write)
		.value("Read", SPIMessage::Direction::Read);
	spiMessage
		.def(pybind11::init())
		.def_readwrite("direction", &SPIMessage::direction)
		.def_readwrite("address", &SPIMessage::address)
		.def_readwrite("mms", &SPIMessage::mms)
		.def_readwrite("stats", &SPIMessage::stats)
		.def_readwrite("payload", &SPIMessage::payload);
}

} // namespace icsneo 

