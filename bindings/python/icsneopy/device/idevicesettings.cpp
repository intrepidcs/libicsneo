#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include "icsneo/device/idevicesettings.h"

#include <fstream>

namespace icsneo {

struct DeviceSettingsNamespace {
    using EthLinkMode = AELinkMode;
    using LinkSpeed = EthLinkSpeed;
};

void init_idevicesettings(pybind11::module_& m) {
    pybind11::classh<DeviceSettingsNamespace> settings(m, "Settings");

    pybind11::enum_<DeviceSettingsNamespace::EthLinkMode>(settings, "EthernetLinkMode")
        .value("Auto", DeviceSettingsNamespace::EthLinkMode::AE_LINK_AUTO)
        .value("Slave", DeviceSettingsNamespace::EthLinkMode::AE_LINK_SLAVE)
        .value("Master", DeviceSettingsNamespace::EthLinkMode::AE_LINK_MASTER);

    pybind11::enum_<DeviceSettingsNamespace::LinkSpeed>(settings, "EthernetLinkSpeed")
        .value("Speed10M", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_10)
        .value("Speed100M", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_100)
        .value("Speed1G", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_1000)
        .value("Speed2_5G", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_2500)
        .value("Speed5G", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_5000)
        .value("Speed10G", DeviceSettingsNamespace::LinkSpeed::ETH_SPEED_10000);

    pybind11::enum_<LINMode>(settings, "LINMode")
        .value("Sleep", LINMode::SLEEP_MODE)
        .value("Slow", LINMode::SLOW_MODE)
        .value("Normal", LINMode::NORMAL_MODE)
        .value("Fast", LINMode::FAST_MODE);

    pybind11::classh<IDeviceSettings>(m, "IDeviceSettings")
        .def("apply", &IDeviceSettings::apply, pybind11::arg("temporary") = 0, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("apply_defaults", &IDeviceSettings::applyDefaults, pybind11::arg("temporary") = 0, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("refresh", &IDeviceSettings::refresh, pybind11::call_guard<pybind11::gil_scoped_release>())
        
        // Baudrate methods
        .def("get_baudrate", &IDeviceSettings::getBaudrateFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_baudrate", &IDeviceSettings::setBaudrateFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_fd_baudrate", &IDeviceSettings::getFDBaudrateFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_fd_baudrate", &IDeviceSettings::setFDBaudrateFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        
        // Termination methods
        .def("is_termination_supported", &IDeviceSettings::isTerminationSupportedFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("can_termination_be_enabled", &IDeviceSettings::canTerminationBeEnabledFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("is_termination_enabled", &IDeviceSettings::isTerminationEnabledFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_termination", &IDeviceSettings::setTerminationFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_termination_groups", &IDeviceSettings::getTerminationGroups, pybind11::call_guard<pybind11::gil_scoped_release>())
        
        // LIN methods
        .def("is_commander_resistor_enabled", &IDeviceSettings::isCommanderResistorEnabledFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_commander_resistor", &IDeviceSettings::setCommanderResistorFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_lin_mode", &IDeviceSettings::getLINModeFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_lin_mode", &IDeviceSettings::setLINModeFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_lin_commander_response_time", &IDeviceSettings::getLINCommanderResponseTimeFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_lin_commander_response_time", &IDeviceSettings::setLINCommanderResponseTimeFor, pybind11::call_guard<pybind11::gil_scoped_release>())
        
        // Ethernet PHY methods
        .def("get_phy_enable", &IDeviceSettings::getPhyEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_phy_mode", &IDeviceSettings::getPhyMode, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_phy_speed", &IDeviceSettings::getPhySpeed, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_enable", &IDeviceSettings::setPhyEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_mode", &IDeviceSettings::setPhyMode, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_speed", &IDeviceSettings::setPhySpeed, pybind11::call_guard<pybind11::gil_scoped_release>())
        
        // Status properties
        .def_readonly("disabled", &IDeviceSettings::disabled)
        .def_readonly("readonly", &IDeviceSettings::readonly);
}

} // namespace icsneo

