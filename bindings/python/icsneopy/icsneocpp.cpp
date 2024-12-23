#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/icsneocpp.h"

namespace icsneo {

void init_event(pybind11::module_&);
void init_eventcallback(pybind11::module_&);
void init_eventmanager(pybind11::module_&);
void init_network(pybind11::module_&);
void init_devicetype(pybind11::module_&);
void init_message(pybind11::module_&);
void init_canmessage(pybind11::module_&);
void init_canerrorcountmessage(pybind11::module_&);
void init_ethernetmessage(pybind11::module_&);
void init_linmessage(pybind11::module_&);
void init_tc10statusmessage(pybind11::module_&);
void init_gptpstatusmessage(pybind11::module_&);
void init_mdiomessage(pybind11::module_&);
void init_device(pybind11::module_&);
void init_messagefilter(pybind11::module_&);
void init_messagecallback(pybind11::module_&);
void init_version(pybind11::module_&);

PYBIND11_MODULE(icsneopy, m) {
	m.doc() = "libicsneo Python module";

	init_event(m);
	init_eventcallback(m);
	init_eventmanager(m);
	init_version(m);
	init_devicetype(m);
	init_network(m);
	init_message(m);
	init_canmessage(m);
	init_canerrorcountmessage(m);
	init_ethernetmessage(m);
	init_linmessage(m);
	init_tc10statusmessage(m);
	init_gptpstatusmessage(m);
	init_mdiomessage(m);
	init_messagefilter(m);
	init_messagecallback(m);
	init_device(m);

	m.def("find_all_devices", &FindAllDevices);
	m.def("get_supported_devices", &GetSupportedDevices);
	m.def("get_last_error", &GetLastError);
}

} // namespace icsneo 
