#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/device/extensions/deviceextension.h"

namespace icsneo {

void init_deviceextension(pybind11::module_& m) {
    pybind11::classh<DeviceExtension>(m, "DeviceExtension")
        .def("get_name", &DeviceExtension::getName);
}

} // namespace icsneo