#include <icsneo/communication/message/apperrormessage.h>

namespace icsneo {

#pragma pack(push, 2)

typedef struct {
	uint16_t error_type;
	uint16_t network_id;
	uint32_t uiTimeStamp10uS;
	uint32_t uiTimeStamp10uSMSB;
} AppErrorData;

#pragma pack(pop)

std::shared_ptr<Message> AppErrorMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	const AppErrorData* data = reinterpret_cast<const AppErrorData*>(bytestream.data());
	if(!data) {
		report(APIEvent::Type::AppErrorParsingFailed, APIEvent::Severity::Error);
		return nullptr;
	}
	auto appErr = std::make_shared<AppErrorMessage>();
	appErr->errorType = data->error_type;
	appErr->errorNetID = static_cast<Network::NetID>(data->network_id);
	appErr->timestamp10us = data->uiTimeStamp10uS;
	appErr->timestamp10usMSB = data->uiTimeStamp10uSMSB;
	appErr->network = Network::NetID::RED_App_Error;
	return appErr;
}

AppErrorType AppErrorMessage::getAppErrorType() {
	AppErrorType errType = static_cast<AppErrorType>(errorType);
	if(errType > AppErrorType::AppNoError) {
		return AppErrorType::AppNoError;
	}
	return errType;
}

std::string AppErrorMessage::getAppErrorString() {
	auto netIDString = Network::GetNetIDString(errorNetID);
	AppErrorType errType = static_cast<AppErrorType>(errorType);
	switch (errType) {
		case AppErrorType::AppErrorRxMessagesFull:
			return std::string(netIDString) + ": RX message buffer full";
		case AppErrorType::AppErrorTxMessagesFull:
			return std::string(netIDString) + ": TX message buffer full";
		case AppErrorType::AppErrorTxReportMessagesFull:
			return std::string(netIDString) + ": TX report buffer full";
		case AppErrorType::AppErrorBadCommWithDspIC:
			return "Received bad packet from DSP IC";
		case AppErrorType::AppErrorDriverOverflow:
			return std::string(netIDString) + ": Driver overflow";
		case AppErrorType::AppErrorPCBuffOverflow:
			return "PC buffer overflow";
		case AppErrorType::AppErrorPCChksumError:
			return "PC checksum error";
		case AppErrorType::AppErrorPCMissedByte:
			return "PC missed byte";
		case AppErrorType::AppErrorPCOverrunError:
			return "PC overrun error";
		case AppErrorType::AppErrorSettingFailure:
			return std::string(netIDString) + ": Settings incorrectly set";
		case AppErrorType::AppErrorTooManySelectedNetworks:
			return "Too many selected networks";
		case AppErrorType::AppErrorNetworkNotEnabled:
			return std::string(netIDString) + ": Network not enabled";
		case AppErrorType::AppErrorRtcNotCorrect:
			return "RTC not correct";
		case AppErrorType::AppErrorLoadedDefaultSettings:
			return "Loaded default settings";
		case AppErrorType::AppErrorFeatureNotUnlocked:
			return "Feature not unlocked";
		case AppErrorType::AppErrorFeatureRtcCmdDropped:
			return "RTC command dropped";
		case AppErrorType::AppErrorTxMessagesFlushed:
			return "TX message buffer flushed";
		case AppErrorType::AppErrorTxMessagesHalfFull:
			return "TX message buffer half full";
		case AppErrorType::AppErrorNetworkNotValid:
			return "Network is not valid";
		case AppErrorType::AppErrorTxInterfaceNotImplemented:
			return "TX interface is not implemented";
		case AppErrorType::AppErrorTxMessagesCommEnableIsOff:
			return "TX message communication is disabled";
		case AppErrorType::AppErrorRxFilterMatchCountExceeded:
			return "RX filter match count exceeded";
		case AppErrorType::AppErrorEthPreemptionNotEnabled:
			return std::string(netIDString) + ": Ethernet preemption not enabled";
		case AppErrorType::AppErrorTxNotSupportedInMode:
			return std::string(netIDString) + ": Transmit is not supported in this mode";
		case AppErrorType::AppErrorJumboFramesNotSupported:
			return std::string(netIDString) + ": Jumbo frames not supported";
		case AppErrorType::AppErrorEthernetIpFragment:
			return "Ethernet IP fragment received";
		case AppErrorType::AppErrorTxMessagesUnderrun:
			return std::string(netIDString) + ": Transmit buffer underrun";
		case AppErrorType::AppErrorDeviceFanFailure:
			return "Device fan failure";
		case AppErrorType::AppErrorDeviceOvertemperature:
			return "Device overtemperature";
		case AppErrorType::AppErrorTxMessageIndexOutOfRange:
			return "Transmit message index out of range";
		case AppErrorType::AppErrorUndersizedFrameDropped:
			return std::string(netIDString) + ": Undersized frame dropped";
		case AppErrorType::AppErrorOversizedFrameDropped:
			return std::string(netIDString) + ": Oversized frame dropped";
		case AppErrorType::AppErrorWatchdogEvent:
			return "Watchdog event occured";
		case AppErrorType::AppErrorSystemClockFailure:
			return "Device clock failed";
		case AppErrorType::AppErrorSystemClockRecovered:
			return "Device clock recovered";
		case AppErrorType::AppErrorSystemPeripheralReset:
			return "Device peripheral reset";
		case AppErrorType::AppErrorSystemCommunicationFailure:
			return "Device communication failure";
		case AppErrorType::AppErrorTxMessagesUnsupportedSourceOrPacketId:
			return std::string(netIDString) + ": Transmit unsupported source or packet ID";
		case AppErrorType::AppErrorWbmsManagerConnectFailed:
			return std::string(netIDString) + ": Failed to connect to managers with settings";
		case AppErrorType::AppErrorWbmsManagerConnectBadState:
			return std::string(netIDString) + ": Connected to managers in a invalid state";
		case AppErrorType::AppErrorWbmsManagerConnectTimeout:
			return std::string(netIDString) + ": Timeout while attempting to connect to managers";
		case AppErrorType::AppErrorFailedToInitializeLoggerDisk:
			return "Device failed to initialize storage disk";
		case AppErrorType::AppErrorInvalidSetting:
			return std::string(netIDString) + ": Invalid settings";
		case AppErrorType::AppErrorSystemFailureRequestedReset:
			return "Device rebooted to recover from an unexpected error condition";
		case AppErrorType::AppErrorPortKeyMistmatch:
			return std::string(netIDString) + ": Mismatch between key in manager and stored key";
		case AppErrorType::AppErrorBusFailure:
			return std::string(netIDString) + ": Bus failure";
		case AppErrorType::AppErrorTapOverflow:
			return std::string(netIDString) + ": Tap overflow";
		case AppErrorType::AppErrorEthTxNoLink:
			return std::string(netIDString) + ": Attempted Ethernet transmit without link";
		case AppErrorType::AppErrorErrorBufferOverflow:
			return "Device error buffer overflow";
		case AppErrorType::AppNoError:
			return "No error";
		default:
			return "Unknown error";
	}
}

} // namespace icsneo