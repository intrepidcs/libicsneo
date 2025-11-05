#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/gptpstatusmessage.h"

namespace icsneo {

void init_gptpstatusmessage(pybind11::module_& m) {
	pybind11::class_<GPTPStatus, std::shared_ptr<GPTPStatus>, Message> gptpStatus(m, "GPTPStatus");
	
	pybind11::class_<GPTPStatus::Timestamp>(gptpStatus, "Timestamp")
		.def_readonly("seconds", &GPTPStatus::Timestamp::seconds)
		.def_readonly("nanoseconds", &GPTPStatus::Timestamp::nanoseconds)
		.def("to_seconds", &GPTPStatus::Timestamp::toSeconds, pybind11::call_guard<pybind11::gil_scoped_release>());
	
	pybind11::class_<GPTPStatus::ScaledNanoSeconds>(gptpStatus, "ScaledNanoSeconds")
		.def_readonly("nanoseconds_msb", &GPTPStatus::ScaledNanoSeconds::nanosecondsMSB)
		.def_readonly("nanoseconds_lsb", &GPTPStatus::ScaledNanoSeconds::nanosecondsLSB)
		.def_readonly("fractional_nanoseconds", &GPTPStatus::ScaledNanoSeconds::fractionalNanoseconds);
	
	pybind11::class_<GPTPStatus::PortID>(gptpStatus, "PortID")
		.def_readonly("clock_identity", &GPTPStatus::PortID::clockIdentity)
		.def_readonly("port_number", &GPTPStatus::PortID::portNumber);
	
	pybind11::class_<GPTPStatus::ClockQuality>(gptpStatus, "ClockQuality")
		.def_readonly("clock_class", &GPTPStatus::ClockQuality::clockClass)
		.def_readonly("clock_accuracy", &GPTPStatus::ClockQuality::clockAccuracy)
		.def_readonly("offset_scaled_log_variance", &GPTPStatus::ClockQuality::offsetScaledLogVariance);
	
	pybind11::class_<GPTPStatus::SystemID>(gptpStatus, "SystemID")
		.def_readonly("priority1", &GPTPStatus::SystemID::priority1)
		.def_readonly("clock_quality", &GPTPStatus::SystemID::clockQuality)
		.def_readonly("priority2", &GPTPStatus::SystemID::priority2)
		.def_readonly("clock_id", &GPTPStatus::SystemID::clockID);
	
	pybind11::class_<GPTPStatus::PriorityVector>(gptpStatus, "PriorityVector")
		.def_readonly("sys_id", &GPTPStatus::PriorityVector::sysID)
		.def_readonly("steps_removed", &GPTPStatus::PriorityVector::stepsRemoved)
		.def_readonly("port_id", &GPTPStatus::PriorityVector::portID)
		.def_readonly("port_number", &GPTPStatus::PriorityVector::portNumber);
	
	pybind11::class_<GPTPStatus::ParentDS>(gptpStatus, "ParentDS")
		.def_readonly("parent_port_identity", &GPTPStatus::ParentDS::parentPortIdentity)
		.def_readonly("cumulative_rate_ratio", &GPTPStatus::ParentDS::cumulativeRateRatio)
		.def_readonly("grandmaster_identity", &GPTPStatus::ParentDS::grandmasterIdentity)
		.def_readonly("gm_clock_quality_clock_class", &GPTPStatus::ParentDS::gmClockQualityClockClass)
		.def_readonly("gm_clock_quality_clock_accuracy", &GPTPStatus::ParentDS::gmClockQualityClockAccuracy)
		.def_readonly("gm_clock_quality_offset_scaled_log_variance", &GPTPStatus::ParentDS::gmClockQualityOffsetScaledLogVariance)
		.def_readonly("gm_priority1", &GPTPStatus::ParentDS::gmPriority1)
		.def_readonly("gm_priority2", &GPTPStatus::ParentDS::gmPriority2);

	pybind11::class_<GPTPStatus::CurrentDS>(gptpStatus, "CurrentDS")
		.def_readonly("steps_removed", &GPTPStatus::CurrentDS::stepsRemoved)
		.def_readonly("offset_from_master", &GPTPStatus::CurrentDS::offsetFromMaster)
		.def_readonly("lastgm_phase_change", &GPTPStatus::CurrentDS::lastgmPhaseChange)
		.def_readonly("lastgm_freq_change", &GPTPStatus::CurrentDS::lastgmFreqChange)
		.def_readonly("gm_time_base_indicator", &GPTPStatus::CurrentDS::gmTimeBaseIndicator)
		.def_readonly("gm_change_count", &GPTPStatus::CurrentDS::gmChangeCount)
		.def_readonly("time_of_lastgm_change_event", &GPTPStatus::CurrentDS::timeOfLastgmChangeEvent)
		.def_readonly("time_of_lastgm_phase_change_event", &GPTPStatus::CurrentDS::timeOfLastgmPhaseChangeEvent)
		.def_readonly("time_of_lastgm_freq_change_event", &GPTPStatus::CurrentDS::timeOfLastgmFreqChangeEvent);

	gptpStatus.def_readonly("current_time", &GPTPStatus::currentTime)
		.def_readonly("gm_priority", &GPTPStatus::gmPriority)
		.def_readonly("ms_offset_ns", &GPTPStatus::msOffsetNs)
		.def_readonly("is_sync", &GPTPStatus::isSync)
		.def_readonly("link_status", &GPTPStatus::linkStatus)
		.def_readonly("link_delay_ns", &GPTPStatus::linkDelayNS)
		.def_readonly("selected_role", &GPTPStatus::selectedRole)
		.def_readonly("as_capable", &GPTPStatus::asCapable)
		.def_readonly("is_syntonized", &GPTPStatus::isSyntonized)
		.def_readonly("last_rx_sync_ts", &GPTPStatus::lastRXSyncTS)
		.def_readonly("current_ds", &GPTPStatus::currentDS)
		.def_readonly("parent_ds", &GPTPStatus::parentDS)
		.def_readonly("short_format", &GPTPStatus::shortFormat);

}

} // namespace icsneo
