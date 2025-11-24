#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include "icsneo/device/device.h"
#include "icsneo/device/extensions/deviceextension.h"

#include <fstream>

namespace icsneo {

void init_device(pybind11::module_& m) {
	pybind11::classh<Device>(m, "Device")
		.def("__repr__", &Device::describe)
		.def("add_message_callback", &Device::addMessageCallback, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("clear_script", &Device::clearScript, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("close", &Device::close, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("describe", &Device::describe)
		.def("disable_message_polling", &Device::disableMessagePolling, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("enable_message_polling", &Device::enableMessagePolling, pybind11::arg("filter") = std::nullopt, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_current_message_count", &Device::getCurrentMessageCount)
		.def("get_digital_io", &Device::getDigitalIO, pybind11::arg("type"), pybind11::arg("number"), pybind11::call_guard<pybind11::gil_scoped_release>())
        .def("get_extension", static_cast<std::shared_ptr<DeviceExtension>(Device::*)(const std::string&) const>(&Device::getExtension)) // This has to be static_casted rather than overload_casted because DeviceExtension is forward declared in device.h
        .def("get_flexray_controllers", &Device::getFlexRayControllers)
		.def("get_gptp_status", &Device::getGPTPStatus, pybind11::arg("timeout") = std::chrono::milliseconds(100), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_messages", [](Device& device) { return device.getMessages(); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_polling_message_limit", &Device::getPollingMessageLimit)
		.def("get_product_name", &Device::getProductName)
		.def("get_rtc", &Device::getRTC, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_script_status", &Device::getScriptStatus, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_serial_number", &Device::getSerialNumber)
		.def("get_serial", &Device::getSerial)
		.def("get_supported_rx_networks", &Device::getSupportedRXNetworks, pybind11::return_value_policy::reference)
		.def("get_supported_tx_networks", &Device::getSupportedTXNetworks, pybind11::return_value_policy::reference)
		.def("get_tc10_status", &Device::getTC10Status, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_type", &Device::getType)
		.def("go_offline", &Device::goOffline, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("go_online", &Device::goOnline, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("is_message_polling_enabled", &Device::isMessagePollingEnabled)
		.def("is_online_supported", &Device::isOnlineSupported)
		.def("is_online", &Device::isOnline)
		.def("is_open", &Device::isOpen)
		.def("open", [](Device& device) { return device.open(); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("prepare_script_load", &Device::prepareScriptLoad, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("remove_message_callback", &Device::removeMessageCallback, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("request_tc10_sleep", &Device::requestTC10Sleep, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("request_tc10_wake", &Device::requestTC10Wake, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_digital_io", pybind11::overload_cast<IO, size_t, bool>(&Device::setDigitalIO), pybind11::arg("type"), pybind11::arg("number"), pybind11::arg("value"), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_polling_message_limit", &Device::setPollingMessageLimit)
		.def("set_rtc", &Device::setRTC, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("start_script", &Device::startScript, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("stop_script", &Device::stopScript, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("supports_tc10", &Device::supportsTC10)
		.def("transmit", pybind11::overload_cast<std::shared_ptr<Frame>>(&Device::transmit), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("upload_coremini", [](Device& device, std::string& path, Disk::MemoryType memType) { std::ifstream ifs(path, std::ios::binary); return device.uploadCoremini(ifs, memType); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("write_macsec_config", &Device::writeMACsecConfig, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("send_eth_phy_msg", &Device::sendEthPhyMsg, pybind11::arg("message"), pybind11::arg("timeout") = std::chrono::milliseconds(50), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_chip_versions", &Device::getChipVersions, pybind11::arg("refreshComponents") = true, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def_readonly("settings", &Device::settings);
}

} // namespace icsneo

