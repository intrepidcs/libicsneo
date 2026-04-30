#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/ethernetmessage.h"

namespace icsneo {

void init_ethernetmessage(pybind11::module_& m) {
	pybind11::classh<EthernetMessage::T1S>(m, "EthernetMessageT1S")
		.def(pybind11::init())
		.def_readwrite("isSymbol", &EthernetMessage::T1S::isSymbol)
		.def_readwrite("isBurst", &EthernetMessage::T1S::isBurst)
		.def_readwrite("txCollision", &EthernetMessage::T1S::txCollision)
		.def_readwrite("isWake", &EthernetMessage::T1S::isWake)
		.def_readwrite("nodeId", &EthernetMessage::T1S::nodeId)
		.def_readwrite("burstCount", &EthernetMessage::T1S::burstCount)
		.def_readwrite("symbolType", &EthernetMessage::T1S::symbolType);
		
	pybind11::classh<EthernetMessage, Frame>(m, "EthernetMessage")
		.def(pybind11::init())
		.def_readwrite("fcs", &EthernetMessage::fcs)
		.def_readwrite("frameTooShort", &EthernetMessage::frameTooShort)
		.def_readwrite("noPadding", &EthernetMessage::noPadding)
		.def_readwrite("fcsVerified", &EthernetMessage::fcsVerified)
		.def_readwrite("txAborted", &EthernetMessage::txAborted)
		.def_readwrite("crcError", &EthernetMessage::crcError)
		.def_readwrite("t1s", &EthernetMessage::t1s)
		.def_readwrite("preemptionFlags", &EthernetMessage::preemptionFlags)
		.def("get_destination_mac", &EthernetMessage::getDestinationMAC)
		.def("get_source_mac", &EthernetMessage::getSourceMAC)
		.def("get_ether_type", &EthernetMessage::getEtherType);
}

} // namespace icsneo 
