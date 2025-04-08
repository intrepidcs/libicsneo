#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/flexray/flexraymessage.h"
#include "icsneo/device/extensions/flexray/symbol.h"
#include "icsneo/device/extensions/flexray/channel.h"
#include "icsneo/device/extensions/flexray/crcstatus.h"

namespace icsneo {

struct FlexRayNamespace {
    using Symbol = icsneo::FlexRay::Symbol;
    using CRCStatus = icsneo::FlexRay::CRCStatus;
    using Channel = icsneo::FlexRay::Channel;
};

void init_flexraymessage(pybind11::module_& m) {
    // dummy class to hold the enums
    pybind11::class_<FlexRayNamespace> flexray(m, "FlexRay");
    // enumerations
    pybind11::enum_<FlexRayNamespace::Symbol>(flexray, "Symbol")
        .value("None", FlexRayNamespace::Symbol::None)
        .value("Unknown", FlexRayNamespace::Symbol::Unknown)
        .value("Wakeup", FlexRayNamespace::Symbol::Wakeup)
        .value("CAS", FlexRayNamespace::Symbol::CAS);
    pybind11::enum_<FlexRayNamespace::CRCStatus>(flexray, "CRCStatus")
        .value("OK", FlexRayNamespace::CRCStatus::OK)
        .value("Error", FlexRayNamespace::CRCStatus::Error)
        .value("NoCRC", FlexRayNamespace::CRCStatus::NoCRC);
    pybind11::enum_<FlexRayNamespace::Channel>(flexray, "Channel")
	    .value("None", FlexRayNamespace::Channel::None)
	    .value("A", FlexRayNamespace::Channel::A)
	    .value("B", FlexRayNamespace::Channel::B)
	    .value("AB", FlexRayNamespace::Channel::AB);
    // read-only until transmit is supported
	pybind11::class_<FlexRayMessage, std::shared_ptr<FlexRayMessage>, Frame>(m, "FlexRayMessage")
		.def(pybind11::init())
		.def_readonly("slotid", &FlexRayMessage::slotid)
		.def_readonly("tsslen", &FlexRayMessage::tsslen)
        .def_readonly("framelen", &FlexRayMessage::framelen)
		.def_readonly("symbol", &FlexRayMessage::symbol)
		.def_readonly("header_crc_status", &FlexRayMessage::headerCRCStatus)
		.def_readonly("header_crc", &FlexRayMessage::headerCRC)
		.def_readonly("frame_crc_status", &FlexRayMessage::crcStatus)
		.def_readonly("frame_crc", &FlexRayMessage::frameCRC)
		.def_readonly("channel", &FlexRayMessage::channel)
        .def_readonly("null_frame", &FlexRayMessage::nullFrame)
        .def_readonly("payload_preamble", &FlexRayMessage::payloadPreamble)
        .def_readonly("sync_frame", &FlexRayMessage::sync)
        .def_readonly("startup_frame", &FlexRayMessage::startup)
        .def_readonly("dynamic_frame", &FlexRayMessage::dynamic)
        .def_readonly("cycle", &FlexRayMessage::cycle);
}

} // namespace icsneo 

