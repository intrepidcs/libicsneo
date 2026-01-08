#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/ethernetmessage.h"

namespace icsneo {

void init_ethernetmessage(pybind11::module_& m) {
	pybind11::classh<MACAddress>(m, "MACAddress")
		.def("to_string", &MACAddress::toString)
		.def("__repr__", &MACAddress::toString);
	
	pybind11::classh<EthernetMessage, Frame>(m, "EthernetMessage")
		.def(pybind11::init())
		.def_readwrite("preemptionEnabled", &EthernetMessage::preemptionEnabled)
		.def_readwrite("preemptionFlags", &EthernetMessage::preemptionFlags)
		.def_readwrite("fcs", &EthernetMessage::fcs)
		.def_readwrite("frameTooShort", &EthernetMessage::frameTooShort)
		.def_readwrite("noPadding", &EthernetMessage::noPadding)
		.def_readwrite("fcsVerified", &EthernetMessage::fcsVerified)
		.def_readwrite("txAborted", &EthernetMessage::txAborted)
		.def_readwrite("crcError", &EthernetMessage::crcError)
		.def_readwrite("isT1S", &EthernetMessage::isT1S)
		.def_readwrite("isT1SSymbol", &EthernetMessage::isT1SSymbol)
		.def_readwrite("isT1SBurst", &EthernetMessage::isT1SBurst)
		.def_readwrite("txCollision", &EthernetMessage::txCollision)
		.def_readwrite("isT1SWake", &EthernetMessage::isT1SWake)
		.def_readwrite("t1sNodeId", &EthernetMessage::t1sNodeId)
		.def_readwrite("t1sBurstCount", &EthernetMessage::t1sBurstCount)
		.def_readwrite("t1sSymbolType", &EthernetMessage::t1sSymbolType)
		.def("get_destination_mac", &EthernetMessage::getDestinationMAC, pybind11::return_value_policy::reference)
		.def("get_source_mac", &EthernetMessage::getSourceMAC, pybind11::return_value_policy::reference)
		.def("get_ether_type", &EthernetMessage::getEtherType);
}

} // namespace icsneo 

