#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/ethphymessage.h"

namespace icsneo {

void init_ethphymessage(pybind11::module_& m) {
	pybind11::classh<Clause22Message>(m, "Clause22Message")
		.def_readwrite("phyAddr", &Clause22Message::phyAddr)
		.def_readwrite("page", &Clause22Message::page)
		.def_readwrite("regAddr", &Clause22Message::regAddr)
		.def_readwrite("regVal", &Clause22Message::regVal);
	pybind11::classh<Clause45Message>(m, "Clause45Message")
		.def_readwrite("port", &Clause45Message::port)
		.def_readwrite("device", &Clause45Message::device)
		.def_readwrite("regAddr", &Clause45Message::regAddr)
		.def_readwrite("regVal", &Clause45Message::regVal);
	pybind11::classh<PhyMessage>(m, "PhyMessage")
		.def(pybind11::init())
		.def_readwrite("Enabled", &PhyMessage::Enabled)
		.def_readwrite("WriteEnable", &PhyMessage::WriteEnable)
		.def_readwrite("Clause45Enable", &PhyMessage::Clause45Enable)
		.def_readwrite("Version", &PhyMessage::Version)
		.def_readwrite("BusIndex", &PhyMessage::BusIndex)
		.def_readwrite("Clause22", &PhyMessage::Clause22)
		.def_readwrite("Clause45", &PhyMessage::Clause45);
	pybind11::classh<EthPhyMessage, Message>(m, "EthPhyMessage")
		.def(pybind11::init())
		.def_readwrite("messages", &EthPhyMessage::messages);
}

} // namespace icsneo
