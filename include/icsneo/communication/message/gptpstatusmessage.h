#ifndef __GPTPSTATUSMESSAGE_H_
#define __GPTPSTATUSMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"
#include "icsneo/api/eventmanager.h"


namespace icsneo {

class GPTPStatus : public Message {
public:
	typedef uint64_t TimeInterval;
	typedef uint64_t ClockID;

	struct Timestamp {
		uint64_t seconds;
		uint32_t nanoseconds;

		double toSeconds() const {
			static constexpr double billion = 1e9;
			return (double)seconds + ((double)nanoseconds) / billion;
		}
	};

	struct ScaledNanoSeconds {
		int16_t nanosecondsMSB; // The most significant bits
		int64_t nanosecondsLSB; // The least significant bits
		int16_t fractionalNanoseconds; // Fractional part
	};

	struct PortID {
		ClockID clockIdentity;
		uint16_t portNumber;
	};
	struct ClockQuality {
		uint8_t clockClass;
		uint8_t clockAccuracy;
		uint16_t offsetScaledLogVariance;
	};

	struct SystemID {
		uint8_t priority1;
		ClockQuality clockQuality;
		uint8_t priority2;
		ClockID clockID;
	};

	struct PriorityVector {
		SystemID sysID;
		uint16_t stepsRemoved;
		PortID portID;
		uint16_t portNumber;
	};

	struct ParentDS {
		PortID parentPortIdentity;
		int32_t cumulativeRateRatio;
		ClockID grandmasterIdentity;
		uint8_t gmClockQualityClockClass;
		uint8_t gmClockQualityClockAccuracy;
		uint16_t gmClockQualityOffsetScaledLogVariance;
		uint8_t gmPriority1;
		uint8_t gmPriority2;
	};

	struct CurrentDS {
		uint16_t stepsRemoved;
		TimeInterval offsetFromMaster;
		ScaledNanoSeconds lastgmPhaseChange;
		double lastgmFreqChange;
		uint16_t gmTimeBaseIndicator;
		uint32_t gmChangeCount;
		uint32_t timeOfLastgmChangeEvent;
		uint32_t timeOfLastgmPhaseChangeEvent;
		uint32_t timeOfLastgmFreqChangeEvent;
	};

	GPTPStatus() : Message(Message::Type::GPTPStatus) {}
	static std::shared_ptr<GPTPStatus> DecodeToMessage(std::vector<uint8_t>& bytes, const device_eventhandler_t& report);

	Timestamp currentTime;
	PriorityVector gmPriority;
	int64_t msOffsetNs;
	uint8_t isSync;
	uint8_t linkStatus;
	int64_t linkDelayNS;
	uint8_t selectedRole;
	uint8_t asCapable;
	uint8_t isSyntonized;
	Timestamp lastRXSyncTS; // t2 in IEEE 1588-2019 Figure-16
	CurrentDS currentDS;
	ParentDS parentDS;
	
	bool shortFormat = false; // Set to true if the above variables weren't set (some firmware versions do not contain all the above variables)
};

}

#endif
#endif
