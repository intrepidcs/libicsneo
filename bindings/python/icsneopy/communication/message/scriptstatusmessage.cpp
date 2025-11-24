#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/scriptstatusmessage.h"

namespace icsneo {

void init_scriptstatusmessage(pybind11::module_& m) {
	pybind11::classh<ScriptStatusMessage, Message>(m, "ScriptStatusMessage")
        .def_readonly("isEncrypted", &ScriptStatusMessage::isEncrypted)
        .def_readonly("isCoreminiRunning", &ScriptStatusMessage::isCoreminiRunning)
        .def_readonly("sectorOverflows", &ScriptStatusMessage::sectorOverflows)
        .def_readonly("numRemainingSectorBuffers", &ScriptStatusMessage::numRemainingSectorBuffers)
        .def_readonly("lastSector", &ScriptStatusMessage::lastSector)
        .def_readonly("readBinSize", &ScriptStatusMessage::readBinSize)
        .def_readonly("minSector", &ScriptStatusMessage::minSector)
        .def_readonly("maxSector", &ScriptStatusMessage::maxSector)
        .def_readonly("currentSector", &ScriptStatusMessage::currentSector)
        .def_readonly("coreminiCreateTime", &ScriptStatusMessage::coreminiCreateTime)
        .def_readonly("fileChecksum", &ScriptStatusMessage::fileChecksum)
        .def_readonly("coreminiVersion", &ScriptStatusMessage::coreminiVersion)
        .def_readonly("coreminiHeaderSize", &ScriptStatusMessage::coreminiHeaderSize)
        .def_readonly("diagnosticErrorCode", &ScriptStatusMessage::diagnosticErrorCode)
        .def_readonly("diagnosticErrorCodeCount", &ScriptStatusMessage::diagnosticErrorCodeCount)
        .def_readonly("maxCoreminiSizeKB", &ScriptStatusMessage::maxCoreminiSizeKB);
}

} // namespace icsneo 

