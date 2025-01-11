#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/canerrorcountmessage.h"

namespace icsneo {

void init_canerrorcountmessage(pybind11::module_& m) {
	pybind11::class_<CANErrorCountMessage, std::shared_ptr<CANErrorCountMessage>, Message>(m, "CANErrorCountMessage")
		.def_readonly("network", &CANErrorCountMessage::network)
		.def_readonly("transmitErrorCount", &CANErrorCountMessage::transmitErrorCount)
		.def_readonly("receiveErrorCount", &CANErrorCountMessage::receiveErrorCount)
		.def_readonly("busOff", &CANErrorCountMessage::busOff);
}

} // namespace icsneo 

