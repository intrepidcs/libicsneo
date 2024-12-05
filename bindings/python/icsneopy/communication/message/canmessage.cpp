#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/canmessage.h"

namespace icsneo {

void init_canmessage(pybind11::module_& m) {
	pybind11::class_<CANMessage, std::shared_ptr<CANMessage>, BusMessage>(m, "CANMessage")
		.def(pybind11::init())
		.def_readwrite("arbid", &CANMessage::arbid)
		.def_readwrite("dlcOnWire", &CANMessage::dlcOnWire)
		.def_readwrite("isRemote", &CANMessage::isRemote)
		.def_readwrite("isExtended", &CANMessage::isExtended)
		.def_readwrite("isCANFD", &CANMessage::isCANFD)
		.def_readwrite("baudrateSwitch", &CANMessage::baudrateSwitch)
		.def_readwrite("errorStateIndicator", &CANMessage::errorStateIndicator);
}

} // namespace icsneo 

