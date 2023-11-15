#ifndef __VSA05_H__
#define __VSA05_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class that holds data for application error records
 */
class VSA05 : public VSA {
public:
	/**
	 * Constructor to convert bytestream into application error record
	 *
	 * @param bytes The bytestream containing the record data
	 */
	VSA05(uint8_t* const bytes);

	/**
	 * Get the timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return The timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp; }

private:
	/**
	 * Perform the checksum on this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	enum class ErrorType : uint16_t {
		NetworkReceiveBufferOverflow = 0,
		NetworkTransmitBufferOverflow = 1,
		NetworkTransmitReportBufferOverflow = 2,
		PeripheralProcessorCommunicationError = 3,
		NetworkPeripheralOverflow = 4,
		CommunicationPacketChecksumError = 6,
		CommunicationPacketDetectedMissingByte = 7,
		FailedToApplySettingsToNetwork = 9,
		EnabledNetworkCountExceedsLicenseCapability = 10,
		NetworkNotEnabled = 11,
		DetectedInvalidTimestamp = 12,
		LoadedDefaultSettings = 13,
		DeviceAttemptedUnsupportedOperation = 14,
		TrasmitBufferFillExceededThreshold = 17,
		TransmitRequestedOnInvalidNetwork = 18,
		TransmitRequestedOnTransmitIncapableNetwork = 19,
		TransmitRequestedWhileControllersInactive = 20,
		FilterMatchesExceedLimit = 21,
		EthernetPreemptionError = 22,
		TransmitWhileControllerModeInvalid = 23,
		FragmentedEthernetIPFrame = 25,
		TransmitBufferUnderrun = 26,
		ActiveCoolingFailureDetected = 27,
		OvertemperatureConditionDetected = 28,
		UndersizedEthernetFrame = 30,
		OversizedEthernetFrame = 31,
		SystemWatchdogEventOcurred = 32,
		SystemClockFailureDetected = 33,
		RecoveredFromSystemClockFailure = 34,
		SystemResetFailedPeripheralComponent = 35,
		FailedToInitializeLoggerDisk = 41,
		AttemptedToApplyInvalidSettingsToNetwork = 42
	} errorType; // Enumerated value indicating the type of error that occurred
	uint16_t errorNetwork; // When applicable, the enumerated network index that the error occurred on
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 7 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA05_H__