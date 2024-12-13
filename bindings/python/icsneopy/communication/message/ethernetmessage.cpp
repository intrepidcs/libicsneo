#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/ethernetmessage.h"

namespace icsneo {

void init_ethernetmessage(pybind11::module_& m) {
	pybind11::class_<MACAddress>(m, "MACAddress")
		.def("to_string", &MACAddress::toString)
		.def("__repr__", &MACAddress::toString);
	
	pybind11::class_<EthernetMessage, std::shared_ptr<EthernetMessage>, Frame>(m, "EthernetMessage")
		.def(pybind11::init())
		.def_readwrite("preemptionEnabled", &EthernetMessage::preemptionEnabled)
		.def_readwrite("preemptionFlags", &EthernetMessage::preemptionFlags)
		.def_readwrite("fcs", &EthernetMessage::fcs)
		.def_readwrite("frameTooShort", &EthernetMessage::frameTooShort)
		.def_readwrite("noPadding", &EthernetMessage::noPadding)
		.def("get_destination_mac", &EthernetMessage::getDestinationMAC, pybind11::return_value_policy::reference)
		.def("get_source_mac", &EthernetMessage::getSourceMAC, pybind11::return_value_policy::reference)
		.def("get_ether_type", &EthernetMessage::getEtherType);
}

} // namespace icsneo 

