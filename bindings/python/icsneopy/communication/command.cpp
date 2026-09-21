#include <pybind11/pybind11.h>
#include <pybind11/native_enum.h>

#include "icsneo/communication/command.h"

namespace icsneo {

void init_command(pybind11::module_& m) {
	pybind11::native_enum<Command>(m, "Command", "enum.IntEnum")
		.value("Main51RxBufferOverflow", Command::Main51RxBufferOverflow)
		.value("Main51StartCmd", Command::Main51StartCmd)
		.value("Main51TxFifoOverflow", Command::Main51TxFifoOverflow)
		.value("Main51BulkInNoData", Command::Main51BulkInNoData)
		.value("Main51SetModeComplete", Command::Main51SetModeComplete)
		.value("Main51ReadEeprom", Command::Main51ReadEeprom)
		.value("Main51CmdDone", Command::Main51CmdDone)
		.value("Main51ErrStatus", Command::Main51ErrStatus)
		.value("Main51ReadSectorBuff", Command::Main51ReadSectorBuff)
		.value("Main51WriteSectorBuff", Command::Main51WriteSectorBuff)
		.value("Main51MmcProcessDone", Command::Main51MmcProcessDone)
		.value("Main51ReInitDone", Command::Main51ReInitDone)
		.value("EnableNetworkCommunication", Command::EnableNetworkCommunication)
		.value("EnableNetworkCommunicationEx", Command::EnableNetworkCommunicationEx)
		.value("KeepAlive", Command::KeepAlive)
		.value("RequestSerialNumber", Command::RequestSerialNumber)
		.value("GetMainVersion", Command::GetMainVersion)
		.value("SetSettings", Command::SetSettings)
		.value("SaveSettings", Command::SaveSettings)
		.value("SetDefaultSettings", Command::SetDefaultSettings)
		.value("ReadSettings", Command::ReadSettings)
		.value("ScriptStatus", Command::ScriptStatus)
		.value("WiVICommand", Command::WiVICommand)
		.value("Extended", Command::Extended)
		.value("ExtendedData", Command::ExtendedData)
		.value("FlexRayControl", Command::FlexRayControl)
		.value("CoreMiniPreload", Command::CoreMiniPreload)
		.finalize();
}

} // namespace icsneo
