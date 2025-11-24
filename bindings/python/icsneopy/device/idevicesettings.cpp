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
    pybind11::class_<DeviceSettingsNamespace> settings(m, "Settings");

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

    pybind11::classh<IDeviceSettings>(m, "IDeviceSettings")
        .def("apply", &IDeviceSettings::apply, pybind11::arg("temporary") = 0, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("apply_defaults", &IDeviceSettings::applyDefaults, pybind11::arg("temporary") = 0, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_phy_enable", &IDeviceSettings::getPhyEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_phy_mode", &IDeviceSettings::getPhyMode, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_phy_speed", &IDeviceSettings::getPhySpeed, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_enable", &IDeviceSettings::setPhyEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_mode", &IDeviceSettings::setPhyMode, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("set_phy_speed", &IDeviceSettings::setPhySpeed, pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("refresh", &IDeviceSettings::refresh, pybind11::call_guard<pybind11::gil_scoped_release>());
}

} // namespace icsneo

