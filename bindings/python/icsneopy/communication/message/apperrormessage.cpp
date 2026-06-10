#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/native_enum.h>

#include "icsneo/communication/message/apperrormessage.h"

namespace icsneo {

void init_errortypes(pybind11::module_& m) {
	pybind11::native_enum<AppErrorType>(m, "AppErrorType", "enum.IntEnum")
		.value("RxMessagesFull", AppErrorType::AppErrorRxMessagesFull)
		.value("TxMessagesFull", AppErrorType::AppErrorTxMessagesFull)
		.value("TxReportMessagesFull", AppErrorType::AppErrorTxReportMessagesFull)
		.value("BadCommWithDspIC", AppErrorType::AppErrorBadCommWithDspIC)
		.value("DriverOverflow", AppErrorType::AppErrorDriverOverflow)
		.value("PCBuffOverflow", AppErrorType::AppErrorPCBuffOverflow)
		.value("PCChksumError", AppErrorType::AppErrorPCChksumError)
		.value("PCMissedByte", AppErrorType::AppErrorPCMissedByte)
		.value("PCOverrunError", AppErrorType::AppErrorPCOverrunError)
		.value("SettingFailure", AppErrorType::AppErrorSettingFailure)
		.value("TooManySelectedNetworks", AppErrorType::AppErrorTooManySelectedNetworks)
		.value("NetworkNotEnabled", AppErrorType::AppErrorNetworkNotEnabled)
		.value("RtcNotCorrect", AppErrorType::AppErrorRtcNotCorrect)
		.value("LoadedDefaultSettings", AppErrorType::AppErrorLoadedDefaultSettings)
		.value("FeatureNotUnlocked", AppErrorType::AppErrorFeatureNotUnlocked)
		.value("FeatureRtcCmdDropped", AppErrorType::AppErrorFeatureRtcCmdDropped)
		.value("TxMessagesFlushed", AppErrorType::AppErrorTxMessagesFlushed)
		.value("TxMessagesHalfFull", AppErrorType::AppErrorTxMessagesHalfFull)
		.value("NetworkNotValid", AppErrorType::AppErrorNetworkNotValid)
		.value("TxInterfaceNotImplemented", AppErrorType::AppErrorTxInterfaceNotImplemented)
		.value("TxMessagesCommEnableIsOff", AppErrorType::AppErrorTxMessagesCommEnableIsOff)
		.value("RxFilterMatchCountExceeded", AppErrorType::AppErrorRxFilterMatchCountExceeded)
		.value("EthPreemptionNotEnabled", AppErrorType::AppErrorEthPreemptionNotEnabled)
		.value("TxNotSupportedInMode", AppErrorType::AppErrorTxNotSupportedInMode)
		.value("JumboFramesNotSupported", AppErrorType::AppErrorJumboFramesNotSupported)
		.value("EthernetIpFragment", AppErrorType::AppErrorEthernetIpFragment)
		.value("TxMessagesUnderrun", AppErrorType::AppErrorTxMessagesUnderrun)
		.value("DeviceFanFailure", AppErrorType::AppErrorDeviceFanFailure)
		.value("DeviceOvertemperature", AppErrorType::AppErrorDeviceOvertemperature)
		.value("TxMessageIndexOutOfRange", AppErrorType::AppErrorTxMessageIndexOutOfRange)
		.value("UndersizedFrameDropped", AppErrorType::AppErrorUndersizedFrameDropped)
		.value("OversizedFrameDropped", AppErrorType::AppErrorOversizedFrameDropped)
		.value("WatchdogEvent", AppErrorType::AppErrorWatchdogEvent)
		.value("SystemClockFailure", AppErrorType::AppErrorSystemClockFailure)
		.value("SystemClockRecovered", AppErrorType::AppErrorSystemClockRecovered)
		.value("SystemPeripheralReset", AppErrorType::AppErrorSystemPeripheralReset)
		.value("SystemCommunicationFailure", AppErrorType::AppErrorSystemCommunicationFailure)
		.value("TxMessagesUnsupportedSourceOrPacketId", AppErrorType::AppErrorTxMessagesUnsupportedSourceOrPacketId)
		.value("WbmsManagerConnectFailed", AppErrorType::AppErrorWbmsManagerConnectFailed)
		.value("WbmsManagerConnectBadState", AppErrorType::AppErrorWbmsManagerConnectBadState)
		.value("WbmsManagerConnectTimeout", AppErrorType::AppErrorWbmsManagerConnectTimeout)
		.value("FailedToInitializeLoggerDisk", AppErrorType::AppErrorFailedToInitializeLoggerDisk)
		.value("InvalidSetting", AppErrorType::AppErrorInvalidSetting)
		.value("SystemFailureRequestedReset", AppErrorType::AppErrorSystemFailureRequestedReset)
		.value("PortKeyMistmatch", AppErrorType::AppErrorPortKeyMistmatch)
		.value("BusFailure", AppErrorType::AppErrorBusFailure)
		.value("TapOverflow", AppErrorType::AppErrorTapOverflow)
		.value("EthTxNoLink", AppErrorType::AppErrorEthTxNoLink)
		.value("ErrorBufferOverflow", AppErrorType::AppErrorErrorBufferOverflow)
		.value("NoError", AppErrorType::AppNoError)
		.finalize();
}

void init_apperrormessage(pybind11::module_& m) {
	init_errortypes(m);
	pybind11::classh<AppErrorMessage, Message>(m, "AppErrorMessage")
		.def("get_app_error_type", &AppErrorMessage::getAppErrorType)
		.def("get_app_error_string", &AppErrorMessage::getAppErrorString);
}

} // namespace icsneo 
