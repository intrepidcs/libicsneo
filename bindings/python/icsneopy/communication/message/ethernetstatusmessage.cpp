#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/ethernetstatusmessage.h"

namespace icsneo {

void init_ethernetstatusmessage(pybind11::module_& m) {
	pybind11::class_<EthernetStatusMessage, std::shared_ptr<EthernetStatusMessage>, Message> ethernetStatusMessage(m, "EthernetStatusMessage");

	pybind11::enum_<EthernetStatusMessage::LinkSpeed>(ethernetStatusMessage, "LinkSpeed")
		.value("LinkSpeedAuto", EthernetStatusMessage::LinkSpeed::LinkSpeedAuto)
		.value("LinkSpeed10", EthernetStatusMessage::LinkSpeed::LinkSpeed10)
		.value("LinkSpeed100", EthernetStatusMessage::LinkSpeed::LinkSpeed100)
		.value("LinkSpeed1000", EthernetStatusMessage::LinkSpeed::LinkSpeed1000)
		.value("LinkSpeed2500", EthernetStatusMessage::LinkSpeed::LinkSpeed2500)
		.value("LinkSpeed5000", EthernetStatusMessage::LinkSpeed::LinkSpeed5000)
		.value("LinkSpeed10000", EthernetStatusMessage::LinkSpeed::LinkSpeed10000);

	pybind11::enum_<EthernetStatusMessage::LinkMode>(ethernetStatusMessage, "LinkMode")
		.value("LinkModeAuto", EthernetStatusMessage::LinkMode::LinkModeAuto)
		.value("LinkModeMaster", EthernetStatusMessage::LinkMode::LinkModeMaster)
		.value("LinkModeSlave", EthernetStatusMessage::LinkMode::LinkModeSlave)
		.value("LinkModeInvalid", EthernetStatusMessage::LinkMode::LinkModeInvalid)
		.value("LinkModeNone", EthernetStatusMessage::LinkMode::LinkModeNone);

	ethernetStatusMessage
		.def_readonly("network", &EthernetStatusMessage::network)
		.def_readonly("state", &EthernetStatusMessage::state)
		.def_readonly("speed", &EthernetStatusMessage::speed)
		.def_readonly("duplex", &EthernetStatusMessage::duplex)
		.def_readonly("mode", &EthernetStatusMessage::mode);
}

} // namespace icsneo
