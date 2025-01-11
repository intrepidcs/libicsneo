#include <icsneo/communication/message/gptpstatusmessage.h>
#include <icsneo/communication/message/extendedresponsemessage.h>
#include <cstring>

namespace icsneo {
typedef double float64;
typedef int64_t time_interval;
typedef uint64_t _clock_identity;


#pragma pack(push, 1)

struct port_identity {
	_clock_identity clock_identity;
	uint16_t port_number;
};

struct _scaled_ns {
	int16_t nanoseconds_msb;
	int64_t nanoseconds_lsb;
	int16_t fractional_nanoseconds;
};

struct _clock_quality {
	uint8_t clock_class;
	uint8_t clock_accuracy;
	uint16_t offset_scaled_log_variance;
};

struct system_identity {
	uint8_t priority_1;
	struct _clock_quality clock_quality;
	uint8_t priority_2;
	_clock_identity clock_identity;
};

struct _timestamp {
	uint16_t seconds_msb;
	uint32_t seconds_lsb;
	uint32_t nanoseconds;
};

struct priority_vector {
	struct system_identity sysid;
	uint16_t steps_removed;
	struct port_identity portid;
	uint16_t port_number;
};


// IEEE 802.1AS-2020 14.3
// This is a read-only data structure
struct _current_ds {
	uint16_t steps_removed;
	time_interval offset_from_master;
	struct _scaled_ns last_gm_phase_change;
	float64 last_gm_freq_change;
	uint16_t gm_time_base_indicator;
	uint32_t gm_change_count;
	uint32_t time_of_last_gm_change_event;
	uint32_t time_of_last_gm_phase_change_event;
	uint32_t time_of_last_gm_freq_change_event;
};

// IEEE 802.1AS-2020 14.4
// This is a read-only data structure
struct _parent_ds {
	struct port_identity parent_port_identity;
	int32_t cumulative_rate_ratio;
	_clock_identity grandmaster_identity;
	uint8_t gm_clock_quality_clock_class;
	uint8_t gm_clock_quality_clock_accuracy;
	uint16_t gm_clock_quality_offset_scaled_log_variance;
	uint8_t gm_priority1;
	uint8_t gm_priority2;
};

struct _GPTPStatus
{
	struct _timestamp current_time;
	struct priority_vector gm_priority;
	int64_t ms_offset_ns;
	uint8_t is_sync;
	uint8_t link_status;
	int64_t link_delay_ns;
	uint8_t selected_role;
	uint8_t as_capable;
	uint8_t is_syntonized;
	struct _timestamp last_rx_sync_ts; // t2 in IEEE 1588-2019 Figure-16
	struct _current_ds current_ds;
	struct _parent_ds parent_ds;
};

#pragma pack(pop)

static void SetField(GPTPStatus::Timestamp& output, const _timestamp& input) {
	output.seconds = ((uint64_t)(input.seconds_msb) << 32) | ((uint64_t)input.seconds_lsb);
	output.nanoseconds = input.nanoseconds;
}

static void SetField(GPTPStatus::PortID& output, const port_identity& input) {
	output.clockIdentity = input.clock_identity;
	output.portNumber = input.port_number;	
}
static void SetField(GPTPStatus::ClockQuality& output, const _clock_quality& input) {
	output.clockClass = input.clock_class;
	output.clockAccuracy = input.clock_accuracy;
	output.offsetScaledLogVariance = input.offset_scaled_log_variance;
}

static void SetField(GPTPStatus::SystemID& output, const system_identity& input) {
	output.priority1 = input.priority_1;
	SetField(output.clockQuality, input.clock_quality);
	output.priority2 = input.priority_2;
	output.clockID = input.clock_identity;
}

static void SetField(GPTPStatus::ScaledNanoSeconds& output, const _scaled_ns& input) {
	output.nanosecondsMSB = input.nanoseconds_msb;
	output.nanosecondsLSB = input.nanoseconds_lsb;
	output.fractionalNanoseconds = input.fractional_nanoseconds;
}

static void SetField(GPTPStatus::PriorityVector& output, const priority_vector& input) {
	SetField(output.sysID, input.sysid);
	output.stepsRemoved = input.steps_removed;
	SetField(output.portID, input.portid);
	output.portNumber = input.port_number;
}

static void SetField(GPTPStatus::CurrentDS& output, const _current_ds& input) {
	output.stepsRemoved = input.steps_removed;
	output.offsetFromMaster = input.offset_from_master;
	SetField(output.lastgmPhaseChange, input.last_gm_phase_change);
	output.lastgmFreqChange = input.last_gm_freq_change;	
	output.gmTimeBaseIndicator = input.gm_time_base_indicator;
	output.gmChangeCount = input.gm_change_count;
	output.timeOfLastgmChangeEvent = input.time_of_last_gm_change_event;
	output.timeOfLastgmPhaseChangeEvent = input.time_of_last_gm_phase_change_event;
	output.timeOfLastgmFreqChangeEvent = input.time_of_last_gm_freq_change_event;
}

static void SetField(GPTPStatus::ParentDS& output, const _parent_ds& input) {
	SetField(output.parentPortIdentity, input.parent_port_identity);
	output.cumulativeRateRatio = input.cumulative_rate_ratio;
	output.grandmasterIdentity = input.grandmaster_identity;
	output.gmClockQualityClockClass = input.gm_clock_quality_clock_class;
	output.gmClockQualityClockAccuracy = input.gm_clock_quality_clock_accuracy;
	output.gmClockQualityOffsetScaledLogVariance = input.gm_clock_quality_offset_scaled_log_variance;
	output.gmPriority1 = input.gm_priority1;
	output.gmPriority2 = input.gm_priority2;
}

[[maybe_unused]] static void SetField(uint8_t& output, const uint8_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(uint16_t& output, const uint16_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(uint32_t& output, const uint32_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(uint64_t& output, const uint64_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(int8_t& output, const int8_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(int16_t& output, const int16_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(int32_t& output, const int32_t& input) {
	output = input;
}

[[maybe_unused]] static void SetField(int64_t& output, const int64_t& input) {
	output = input;
}

std::shared_ptr<GPTPStatus> GPTPStatus::DecodeToMessage(std::vector<uint8_t>& bytes, const device_eventhandler_t&) {

	// The following does not lead to overflow since we only call this function if it has at least ResponseHeader length bytes
	std::shared_ptr<GPTPStatus> res = std::make_shared<GPTPStatus>();
	auto* header = reinterpret_cast<ExtendedResponseMessage::ResponseHeader*>(bytes.data());
	uint16_t length = header->length;
	_GPTPStatus* input = reinterpret_cast<_GPTPStatus*>(bytes.data() + sizeof(ExtendedResponseMessage::ResponseHeader));

#define CheckLengthAndSet(output, input) if(length >= sizeof(decltype(input))) { \
		SetField(output, input); \
		length -= sizeof(decltype(input)); \
	} else {\
		memset(&output, 0, sizeof(decltype(output))); \
		length = 0; \
		res->shortFormat = true; \
	}

	CheckLengthAndSet(res->currentTime, input->current_time);
	CheckLengthAndSet(res->gmPriority, input->gm_priority);
	CheckLengthAndSet(res->msOffsetNs, input->ms_offset_ns);
	CheckLengthAndSet(res->isSync, input->is_sync);
	CheckLengthAndSet(res->linkStatus, input->link_status);
	CheckLengthAndSet(res->linkDelayNS, input->link_delay_ns);
	CheckLengthAndSet(res->selectedRole, input->selected_role);
	CheckLengthAndSet(res->asCapable, input->as_capable);
	CheckLengthAndSet(res->isSyntonized, input->is_syntonized);
	CheckLengthAndSet(res->lastRXSyncTS, input->last_rx_sync_ts);
	CheckLengthAndSet(res->currentDS, input->current_ds);
	CheckLengthAndSet(res->parentDS, input->parent_ds);

	return res;
}

}