#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/io.h"

namespace icsneo {

void init_io(pybind11::module_& m) {
	pybind11::enum_<IO>(m, "IO")
		.value("EthernetActivation", IO::EthernetActivation)
		.value("USBHostPower", IO::USBHostPower)
		.value("BackupPowerEnabled", IO::BackupPowerEnabled)
		.value("BackupPowerGood", IO::BackupPowerGood)
		.value("Misc", IO::Misc)
		.value("EMisc", IO::EMisc);
}

} // namespace icsneo
