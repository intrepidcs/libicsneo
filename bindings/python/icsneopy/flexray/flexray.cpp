#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/flexray/flexraymessage.h"
#include "icsneo/device/extensions/flexray/symbol.h"
#include "icsneo/device/extensions/flexray/channel.h"
#include "icsneo/device/extensions/flexray/crcstatus.h"
#include "icsneo/device/extensions/flexray/controller.h"

namespace icsneo {

struct FlexRayNamespace {
    using Symbol = icsneo::FlexRay::Symbol;
    using CRCStatus = icsneo::FlexRay::CRCStatus;
    using Channel = icsneo::FlexRay::Channel;
};

namespace FlexRay {

struct ClusterNamespace {
    using SpeedType = icsneo::FlexRay::Cluster::SpeedType;
    using SPPType = icsneo::FlexRay::Cluster::SPPType;
};

void init_extension(pybind11::classh<FlexRayNamespace>& c) {
    pybind11::classh<MessageBuffer>(c, "MessageBuffer")
        .def(pybind11::init())
        .def_readwrite("is_dynamic", &MessageBuffer::isDynamic)
        .def_readwrite("is_sync", &MessageBuffer::isSync)
        .def_readwrite("is_startup", &MessageBuffer::isStartup)
        .def_readwrite("is_network_management_frame", &MessageBuffer::isNetworkManagementFrame)
        .def_readwrite("is_transmit", &MessageBuffer::isTransmit)
        .def_readwrite("frame_id", &MessageBuffer::frameID)
        .def_readwrite("channel_a", &MessageBuffer::channelA)
        .def_readwrite("channel_b", &MessageBuffer::channelB)
        .def_readwrite("frame_length_bytes", &MessageBuffer::frameLengthBytes)
        .def_readwrite("base_cycle", &MessageBuffer::baseCycle)
        .def_readwrite("cycle_repetition", &MessageBuffer::cycleRepetition)
        .def_readwrite("continuous_mode", &MessageBuffer::continuousMode);

    auto controller = pybind11::classh<Controller>(c, "Controller")
        .def("get_network", &Controller::getNetwork)
        .def("get_configuration", &Controller::getConfiguration)
        .def("set_configuration", &Controller::setConfiguration)
        .def("get_start_when_going_online", &Controller::getStartWhenGoingOnline)
        .def("set_start_when_going_online", &Controller::setStartWhenGoingOnline)
        .def("get_allow_coldstart", &Controller::getAllowColdstart)
        .def("set_allow_coldstart", &Controller::setAllowColdstart)
        .def("get_wakeup_before_start", &Controller::getWakeupBeforeStart)
        .def("set_wakeup_before_start", &Controller::setWakeupBeforeStart)
        .def("add_message_buffer", &Controller::addMessageBuffer)
        .def("clear_message_buffers", &Controller::clearMessageBuffers)
        .def("wakeup", &Controller::wakeup)
        .def("configure", &Controller::getReady)
        .def("start", &Controller::start)
        .def("transmit", &Controller::transmit)
        .def("halt", &Controller::halt)
        .def("freeze", &Controller::freeze)
        .def("trigger_mts", &Controller::triggerMTS);

    pybind11::classh<Controller::Configuration>(controller, "Configuration")
        .def(pybind11::init())
        .def_readwrite("accept_startup_range_microticks", &Controller::Configuration::AcceptStartupRangeMicroticks)
        .def_readwrite("allow_passive_to_active_cycle_pairs", &Controller::Configuration::AllowPassiveToActiveCyclePairs)
        .def_readwrite("cluster_drift_damping", &Controller::Configuration::ClusterDriftDamping)
        .def_readwrite("channel_a", &Controller::Configuration::ChannelA)
        .def_readwrite("channel_b", &Controller::Configuration::ChannelB)
        .def_readwrite("decoding_correction_microticks", &Controller::Configuration::DecodingCorrectionMicroticks)
        .def_readwrite("delay_compensation_a_microticks", &Controller::Configuration::DelayCompensationAMicroticks)
        .def_readwrite("delay_compensation_b_microticks", &Controller::Configuration::DelayCompensationBMicroticks)
        .def_readwrite("extern_offset_correction_control", &Controller::Configuration::ExternOffsetCorrectionControl)
        .def_readwrite("extern_rate_correction_control", &Controller::Configuration::ExternRateCorrectionControl)
        .def_readwrite("extern_offset_correction_microticks", &Controller::Configuration::ExternOffsetCorrectionMicroticks)
        .def_readwrite("extern_rate_correction_microticks", &Controller::Configuration::ExternRateCorrectionMicroticks)
        .def_readwrite("key_slot_id", &Controller::Configuration::KeySlotID)
        .def_readwrite("key_slot_used_for_startup", &Controller::Configuration::KeySlotUsedForStartup)
        .def_readwrite("key_slot_used_for_sync", &Controller::Configuration::KeySlotUsedForSync)
        .def_readwrite("latest_tx_minislot", &Controller::Configuration::LatestTxMinislot)
        .def_readwrite("listen_timeout", &Controller::Configuration::ListenTimeout)
        .def_readwrite("macro_initial_offset_a", &Controller::Configuration::MacroInitialOffsetA)
        .def_readwrite("macro_initial_offset_b", &Controller::Configuration::MacroInitialOffsetB)
        .def_readwrite("micro_initial_offset_a", &Controller::Configuration::MicroInitialOffsetA)
        .def_readwrite("micro_initial_offset_b", &Controller::Configuration::MicroInitialOffsetB)
        .def_readwrite("micro_per_cycle", &Controller::Configuration::MicroPerCycle)
        .def_readwrite("mts_on_a", &Controller::Configuration::MTSOnA)
        .def_readwrite("mts_on_b", &Controller::Configuration::MTSOnB)
        .def_readwrite("offset_correction_out_microticks", &Controller::Configuration::OffsetCorrectionOutMicroticks)
        .def_readwrite("rate_correction_out_microticks", &Controller::Configuration::RateCorrectionOutMicroticks)
        .def_readwrite("second_key_slot_id", &Controller::Configuration::SecondKeySlotID)
        .def_readwrite("two_key_slot_mode", &Controller::Configuration::TwoKeySlotMode)
        .def_readwrite("wakeup_pattern", &Controller::Configuration::WakeupPattern)
        .def_readwrite("wakeup_on_channel_b", &Controller::Configuration::WakeupOnChannelB);

    // Dummy class for cluster namespace
    pybind11::classh<ClusterNamespace> cluster(c, "Cluster");

    pybind11::enum_<Cluster::SpeedType>(cluster, "SpeedType")
        .value("FLEXRAY_BAUDRATE_10M", Cluster::SpeedType::FLEXRAY_BAUDRATE_10M)
        .value("FLEXRAY_BAUDRATE_5M", Cluster::SpeedType::FLEXRAY_BAUDRATE_5M)
        .value("FLEXRAY_BAUDRATE_2M5", Cluster::SpeedType::FLEXRAY_BAUDRATE_2M5)
        .value("FLEXRAY_BAUDRATE_2M5_ALT", Cluster::SpeedType::FLEXRAY_BAUDRATE_2M5_ALT);

    pybind11::enum_<Cluster::SPPType>(cluster, "SPPType")
        .value("FLEXRAY_SPP_5", Cluster::SPPType::FLEXRAY_SPP_5)
        .value("FLEXRAY_SPP_4", Cluster::SPPType::FLEXRAY_SPP_4)
        .value("FLEXRAY_SPP_6", Cluster::SPPType::FLEXRAY_SPP_6)
        .value("FLEXRAY_SPP_5_ALT", Cluster::SPPType::FLEXRAY_SPP_5_ALT);

    pybind11::classh<Cluster::Configuration>(cluster, "Configuration")
        .def(pybind11::init())
        .def_readwrite("speed", &Cluster::Configuration::Speed)
        .def_readwrite("strobe_point_position", &Cluster::Configuration::StrobePointPosition)
        .def_readwrite("action_point_offset", &Cluster::Configuration::ActionPointOffset)
        .def_readwrite("casr_x_low_max", &Cluster::Configuration::CASRxLowMax)
        .def_readwrite("cold_start_attempts", &Cluster::Configuration::ColdStartAttempts)
        .def_readwrite("dynamic_slot_idle_phase_minislots", &Cluster::Configuration::DynamicSlotIdlePhaseMinislots)
        .def_readwrite("listen_noise_macroticks", &Cluster::Configuration::ListenNoiseMacroticks)
        .def_readwrite("macroticks_per_cycle", &Cluster::Configuration::MacroticksPerCycle)
        .def_readwrite("macrotick_duration_micro_sec", &Cluster::Configuration::MacrotickDurationMicroSec)
        .def_readwrite("max_without_clock_correction_fatal", &Cluster::Configuration::MaxWithoutClockCorrectionFatal)
        .def_readwrite("max_without_clock_correction_passive", &Cluster::Configuration::MaxWithoutClockCorrectionPassive)
        .def_readwrite("minislot_action_point_offset_macroticks", &Cluster::Configuration::MinislotActionPointOffsetMacroticks)
        .def_readwrite("minislot_duration_macroticks", &Cluster::Configuration::MinislotDurationMacroticks)
        .def_readwrite("network_idle_time_macroticks", &Cluster::Configuration::NetworkIdleTimeMacroticks)
        .def_readwrite("network_management_vector_length_bytes", &Cluster::Configuration::NetworkManagementVectorLengthBytes)
        .def_readwrite("number_of_minislots", &Cluster::Configuration::NumberOfMinislots)
        .def_readwrite("number_of_static_slots", &Cluster::Configuration::NumberOfStaticSlots)
        .def_readwrite("offset_correction_start_macroticks", &Cluster::Configuration::OffsetCorrectionStartMacroticks)
        .def_readwrite("payload_length_of_static_slot_in_words", &Cluster::Configuration::PayloadLengthOfStaticSlotInWords)
        .def_readwrite("static_slot_macroticks", &Cluster::Configuration::StaticSlotMacroticks)
        .def_readwrite("symbol_window_macroticks", &Cluster::Configuration::SymbolWindowMacroticks)
        .def_readwrite("symbol_window_action_point_offset_macroticks", &Cluster::Configuration::SymbolWindowActionPointOffsetMacroticks)
        .def_readwrite("sync_frame_id_count_max", &Cluster::Configuration::SyncFrameIDCountMax)
        .def_readwrite("transmission_start_sequence_duration_bits", &Cluster::Configuration::TransmissionStartSequenceDurationBits)
        .def_readwrite("wakeup_rx_idle_bits", &Cluster::Configuration::WakeupRxIdleBits)
        .def_readwrite("wakeup_rx_low_bits", &Cluster::Configuration::WakeupRxLowBits)
        .def_readwrite("wakeup_rx_window_bits", &Cluster::Configuration::WakeupRxWindowBits)
        .def_readwrite("wakeup_tx_active_bits", &Cluster::Configuration::WakeupTxActiveBits)
        .def_readwrite("wakeup_tx_idle_bits", &Cluster::Configuration::WakeupTxIdleBits);
}

} // namespace FlexRay

void init_flexraymessage(pybind11::module_& m) {
	pybind11::classh<FlexRayMessage, Frame>(m, "FlexRayMessage")
		.def(pybind11::init())
		.def_readwrite("slotid", &FlexRayMessage::slotid)
		.def_readwrite("tsslen", &FlexRayMessage::tsslen)
        .def_readwrite("framelen", &FlexRayMessage::framelen)
		.def_readwrite("symbol", &FlexRayMessage::symbol)
		.def_readwrite("header_crc_status", &FlexRayMessage::headerCRCStatus)
		.def_readwrite("header_crc", &FlexRayMessage::headerCRC)
		.def_readwrite("frame_crc_status", &FlexRayMessage::crcStatus)
		.def_readwrite("frame_crc", &FlexRayMessage::frameCRC)
		.def_readwrite("channel", &FlexRayMessage::channel)
        .def_readwrite("null_frame", &FlexRayMessage::nullFrame)
        .def_readwrite("payload_preamble", &FlexRayMessage::payloadPreamble)
        .def_readwrite("sync_frame", &FlexRayMessage::sync)
        .def_readwrite("startup_frame", &FlexRayMessage::startup)
        .def_readwrite("dynamic_frame", &FlexRayMessage::dynamic)
        .def_readwrite("cycle", &FlexRayMessage::cycle);

    //// TODO: Eliminate FlexRayControlMessage class references in controller class and eliminate getStatus function in bindings
}

void init_flexray(pybind11::module_& m) {
    // Dummy class to act as FlexRay namespace
    pybind11::classh<FlexRayNamespace> flexray(m, "FlexRay");
    
    pybind11::enum_<FlexRayNamespace::Symbol>(flexray, "Symbol")
        .value("None", FlexRayNamespace::Symbol::None)
        .value("Unknown", FlexRayNamespace::Symbol::Unknown)
        .value("Wakeup", FlexRayNamespace::Symbol::Wakeup)
        .value("CAS", FlexRayNamespace::Symbol::CAS);
    pybind11::enum_<FlexRayNamespace::CRCStatus>(flexray, "CRCStatus")
        .value("OK", FlexRayNamespace::CRCStatus::OK)
        .value("Error", FlexRayNamespace::CRCStatus::Error)
        .value("NoCRC", FlexRayNamespace::CRCStatus::NoCRC);
    pybind11::enum_<FlexRayNamespace::Channel>(flexray, "Channel")
	    .value("None", FlexRayNamespace::Channel::None)
	    .value("A", FlexRayNamespace::Channel::A)
	    .value("B", FlexRayNamespace::Channel::B)
	    .value("AB", FlexRayNamespace::Channel::AB);
    
    init_flexraymessage(m);
    FlexRay::init_extension(flexray);
}

} // namespace icsneo 

