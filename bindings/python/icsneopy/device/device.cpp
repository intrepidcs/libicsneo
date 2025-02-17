#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include "icsneo/device/device.h"

namespace icsneo {

void init_device(pybind11::module_& m) {
	pybind11::class_<Device, std::shared_ptr<Device>>(m, "Device")
		.def("get_type", &Device::getType)
		.def("get_serial", &Device::getSerial)
		.def("get_serial_number", &Device::getSerialNumber)
		.def("get_product_name", &Device::getProductName)
		.def("open", [](Device& device) { return device.open(); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("close", &Device::close, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("is_open", &Device::isOpen)
		.def("go_online", &Device::goOnline, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("go_offline", &Device::goOffline, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("is_online", &Device::isOnline)
		.def("enable_message_polling", &Device::enableMessagePolling, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("disable_message_polling", &Device::disableMessagePolling, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("is_message_polling_enabled", &Device::isMessagePollingEnabled)
		.def("get_messages", [](Device& device) { return device.getMessages(); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_current_message_count", &Device::getCurrentMessageCount)
		.def("get_polling_message_limit", &Device::getPollingMessageLimit)
		.def("set_polling_message_limit", &Device::setPollingMessageLimit)
		.def("add_message_callback", &Device::addMessageCallback, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("remove_message_callback", &Device::removeMessageCallback, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("transmit", pybind11::overload_cast<std::shared_ptr<Frame>>(&Device::transmit), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_supported_rx_networks", &Device::getSupportedRXNetworks, pybind11::return_value_policy::reference)
		.def("get_supported_tx_networks", &Device::getSupportedTXNetworks, pybind11::return_value_policy::reference)
		.def("get_rtc", &Device::getRTC, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_rtc", &Device::setRTC, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("describe", &Device::describe)
		.def("is_online_supported", &Device::isOnlineSupported)
		.def("supports_tc10", &Device::supportsTC10)
		.def("request_tc10_wake", &Device::requestTC10Wake, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("request_tc10_sleep", &Device::requestTC10Sleep, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_tc10_status", &Device::getTC10Status, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_gptp_status", &Device::getGPTPStatus, pybind11::arg("timeout") = std::chrono::milliseconds(100), pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("write_macsec_config", &Device::writeMACsecConfig)
		.def("__repr__", &Device::describe);
}

} // namespace icsneo

