#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/native_enum.h>

#include "icsneo/communication/message/canerrormessage.h"

namespace icsneo {

void init_errorcodes(pybind11::module_& m) {
	pybind11::native_enum<CANErrorCode>(m, "CANErrorCode", "enum.IntEnum")
		.value("NoError", CANErrorCode::NoError)
		.value("StuffError", CANErrorCode::StuffError)
		.value("FormError", CANErrorCode::FormError)
		.value("AckError", CANErrorCode::AckError)
		.value("Bit1Error", CANErrorCode::Bit1Error)
		.value("Bit0Error", CANErrorCode::Bit0Error)
		.value("CRCError", CANErrorCode::CRCError)
		.value("NoChange", CANErrorCode::NoChange)
		.finalize();
}

void init_canerrormessage(pybind11::module_& m) {
	init_errorcodes(m);
	pybind11::classh<CANErrorMessage, Message>(m, "CANErrorMessage")
		.def_readonly("network", &CANErrorMessage::network)
		.def_readonly("transmitErrorCount", &CANErrorMessage::transmitErrorCount)
		.def_readonly("receiveErrorCount", &CANErrorMessage::receiveErrorCount)
		.def_readonly("busOff", &CANErrorMessage::busOff)
		.def_readonly("errorPassive", &CANErrorMessage::errorPassive)
		.def_readonly("errorWarn", &CANErrorMessage::errorWarn)
		.def_readonly("dataErrorCode", &CANErrorMessage::dataErrorCode)
		.def_readonly("errorCode", &CANErrorMessage::errorCode);
	

	m.attr("CANErrorCountMessage") = m.attr("CANErrorMessage");
}

} // namespace icsneo 

