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
		.def_readonly("nanosecondsMSB", &GPTPStatus::ScaledNanoSeconds::nanosecondsMSB)
		.def_readonly("nanosecondsLSB", &GPTPStatus::ScaledNanoSeconds::nanosecondsLSB)
		.def_readonly("fractionalNanoseconds", &GPTPStatus::ScaledNanoSeconds::fractionalNanoseconds);
	
	pybind11::class_<GPTPStatus::PortID>(gptpStatus, "PortID")
		.def_readonly("clockIdentity", &GPTPStatus::PortID::clockIdentity)
		.def_readonly("portNumber", &GPTPStatus::PortID::portNumber);
	
	pybind11::class_<GPTPStatus::ClockQuality>(gptpStatus, "ClockQuality")
		.def_readonly("clockClass", &GPTPStatus::ClockQuality::clockClass)
		.def_readonly("clockAccuracy", &GPTPStatus::ClockQuality::clockAccuracy)
		.def_readonly("offsetScaledLogVariance", &GPTPStatus::ClockQuality::offsetScaledLogVariance);
	
	pybind11::class_<GPTPStatus::SystemID>(gptpStatus, "SystemID")
		.def_readonly("priority1", &GPTPStatus::SystemID::priority1)
		.def_readonly("clockQuality", &GPTPStatus::SystemID::clockQuality)
		.def_readonly("priority2", &GPTPStatus::SystemID::priority2)
		.def_readonly("clockID", &GPTPStatus::SystemID::clockID);
	
	pybind11::class_<GPTPStatus::PriorityVector>(gptpStatus, "PriorityVector")
		.def_readonly("sysID", &GPTPStatus::PriorityVector::sysID)
		.def_readonly("stepsRemoved", &GPTPStatus::PriorityVector::stepsRemoved)
		.def_readonly("portID", &GPTPStatus::PriorityVector::portID)
		.def_readonly("portNumber", &GPTPStatus::PriorityVector::portNumber);
	
	pybind11::class_<GPTPStatus::ParentDS>(gptpStatus, "ParentDS")
		.def_readonly("parentPortIdentity", &GPTPStatus::ParentDS::parentPortIdentity)
		.def_readonly("cumulativeRateRatio", &GPTPStatus::ParentDS::cumulativeRateRatio)
		.def_readonly("grandmasterIdentity", &GPTPStatus::ParentDS::grandmasterIdentity)
		.def_readonly("gmClockQualityClockClass", &GPTPStatus::ParentDS::gmClockQualityClockClass)
		.def_readonly("gmClockQualityClockAccuracy", &GPTPStatus::ParentDS::gmClockQualityClockAccuracy)
		.def_readonly("gmClockQualityOffsetScaledLogVariance", &GPTPStatus::ParentDS::gmClockQualityOffsetScaledLogVariance)
		.def_readonly("gmPriority1", &GPTPStatus::ParentDS::gmPriority1)
		.def_readonly("gmPriority2", &GPTPStatus::ParentDS::gmPriority2);

	pybind11::class_<GPTPStatus::CurrentDS>(gptpStatus, "CurrentDS")
		.def_readonly("stepsRemoved", &GPTPStatus::CurrentDS::stepsRemoved)
		.def_readonly("offsetFromMaster", &GPTPStatus::CurrentDS::offsetFromMaster)
		.def_readonly("lastgmPhaseChange", &GPTPStatus::CurrentDS::lastgmPhaseChange)
		.def_readonly("lastgmFreqChange", &GPTPStatus::CurrentDS::lastgmFreqChange)
		.def_readonly("gmTimeBaseIndicator", &GPTPStatus::CurrentDS::gmTimeBaseIndicator)
		.def_readonly("gmChangeCount", &GPTPStatus::CurrentDS::gmChangeCount)
		.def_readonly("timeOfLastgmChangeEvent", &GPTPStatus::CurrentDS::timeOfLastgmChangeEvent)
		.def_readonly("timeOfLastgmPhaseChangeEvent", &GPTPStatus::CurrentDS::timeOfLastgmPhaseChangeEvent)
		.def_readonly("timeOfLastgmFreqChangeEvent", &GPTPStatus::CurrentDS::timeOfLastgmFreqChangeEvent);

	gptpStatus.def_readonly("currentTime", &GPTPStatus::currentTime)
		.def_readonly("gmPriority", &GPTPStatus::gmPriority)
		.def_readonly("msOffsetNs", &GPTPStatus::msOffsetNs)
		.def_readonly("isSync", &GPTPStatus::isSync)
		.def_readonly("linkStatus", &GPTPStatus::linkStatus)
		.def_readonly("linkDelayNS", &GPTPStatus::linkDelayNS)
		.def_readonly("selectedRole", &GPTPStatus::selectedRole)
		.def_readonly("asCapable", &GPTPStatus::asCapable)
		.def_readonly("isSyntonized", &GPTPStatus::isSyntonized)
		.def_readonly("lastRXSyncTS", &GPTPStatus::lastRXSyncTS)
		.def_readonly("currentDS", &GPTPStatus::currentDS)
		.def_readonly("parentDS", &GPTPStatus::parentDS);

}

} // namespace icsneo
