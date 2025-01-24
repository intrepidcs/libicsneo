#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/mdiomessage.h"

namespace icsneo {

void init_mdiomessage(pybind11::module_& m) {
	pybind11::class_<MDIOMessage, std::shared_ptr<MDIOMessage>, BusMessage> mdioMessage(m, "MDIOMessage");
	pybind11::enum_<MDIOMessage::Clause>(mdioMessage, "Clause")
		.value("Clause45", MDIOMessage::Clause::Clause45)
		.value("Clause22", MDIOMessage::Clause::Clause22);
	pybind11::enum_<MDIOMessage::Direction>(mdioMessage, "Direction")
		.value("Write", MDIOMessage::Direction::Write)
		.value("Read", MDIOMessage::Direction::Read);
	mdioMessage
		.def(pybind11::init())
		.def_readwrite("isTXMsg", &MDIOMessage::isTXMsg)
		.def_readwrite("txTimeout", &MDIOMessage::txTimeout)
		.def_readwrite("txAborted", &MDIOMessage::txAborted)
		.def_readwrite("txInvalidBus", &MDIOMessage::txInvalidBus)
		.def_readwrite("txInvalidPhyAddr", &MDIOMessage::txInvalidPhyAddr)
		.def_readwrite("txInvalidRegAddr", &MDIOMessage::txInvalidRegAddr)
		.def_readwrite("txInvalidClause", &MDIOMessage::txInvalidClause)
		.def_readwrite("txInvalidOpcode", &MDIOMessage::txInvalidOpcode)
		.def_readwrite("phyAddress", &MDIOMessage::phyAddress)
		.def_readwrite("devAddress", &MDIOMessage::devAddress)
		.def_readwrite("regAddress", &MDIOMessage::regAddress)
		.def_readwrite("direction", &MDIOMessage::direction)
		.def_readwrite("clause", &MDIOMessage::clause);
}

} // namespace icsneo 

