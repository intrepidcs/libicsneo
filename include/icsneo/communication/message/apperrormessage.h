#ifndef __APPERRORMESSAGE_H_
#define __APPERRORMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include <unordered_set>
#include <memory>
#include "icsneo/api/eventmanager.h"


namespace icsneo {

enum class AppErrorType : uint16_t {
	AppErrorRxMessagesFull = 0,
	AppErrorTxMessagesFull = 1,
	AppErrorTxReportMessagesFull = 2,
	AppErrorBadCommWithDspIC = 3,
	AppErrorDriverOverflow = 4,
	AppErrorPCBuffOverflow = 5,
	AppErrorPCChksumError = 6,
	AppErrorPCMissedByte = 7,
	AppErrorPCOverrunError = 8,
	AppErrorSettingFailure = 9,
	AppErrorTooManySelectedNetworks = 10,
	AppErrorNetworkNotEnabled = 11,
	AppErrorRtcNotCorrect = 12,
	AppErrorLoadedDefaultSettings = 13,
	AppErrorFeatureNotUnlocked = 14,
	AppErrorFeatureRtcCmdDropped = 15,
	AppErrorTxMessagesFlushed = 16,
	AppErrorTxMessagesHalfFull = 17,
	AppErrorNetworkNotValid = 18,
	AppErrorTxInterfaceNotImplemented = 19,
	AppErrorTxMessagesCommEnableIsOff = 20,
	AppErrorRxFilterMatchCountExceeded = 21,
	AppErrorEthPreemptionNotEnabled = 22,
	AppErrorTxNotSupportedInMode = 23,
	AppErrorJumboFramesNotSupported = 24,
	AppErrorEthernetIpFragment = 25,
	AppErrorTxMessagesUnderrun = 26,
	AppErrorDeviceFanFailure = 27,
	AppErrorDeviceOvertemperature = 28,
	AppErrorTxMessageIndexOutOfRange = 29,
	AppErrorUndersizedFrameDropped = 30,
	AppErrorOversizedFrameDropped = 31,
	AppErrorWatchdogEvent = 32,
	AppErrorSystemClockFailure = 33,
	AppErrorSystemClockRecovered = 34,
	AppErrorSystemPeripheralReset = 35,
	AppErrorSystemCommunicationFailure = 36,
	AppErrorTxMessagesUnsupportedSourceOrPacketId = 37,
	AppErrorWbmsManagerConnectFailed = 38,
	AppErrorWbmsManagerConnectBadState = 39,
	AppErrorWbmsManagerConnectTimeout = 40,
	AppErrorFailedToInitializeLoggerDisk = 41,
	AppErrorInvalidSetting = 42,
	AppErrorSystemFailureRequestedReset = 43,
	AppErrorPortKeyMistmatch = 45,
	AppErrorErrorBufferOverflow = 254,
	AppNoError = 255
};

class AppErrorMessage : public RawMessage {
public:
	AppErrorMessage() : RawMessage(Message::Type::AppError, Network::NetID::RED_App_Error) {}
	uint16_t errorType;
	Network::NetID errorNetID;
	uint32_t timestamp10us;
	uint32_t timestamp10usMSB;

	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
	AppErrorType getAppErrorType();
	std::string getAppErrorString();
};

} // namespace icsneo

#endif // __cplusplus
#endif