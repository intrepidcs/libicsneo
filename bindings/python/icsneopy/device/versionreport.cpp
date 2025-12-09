#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/device/versionreport.h"

namespace icsneo {

void init_versionreport(pybind11::module_& m) {
	pybind11::classh<VersionReport>(m, "VersionReport")
		.def_readonly("id", &VersionReport::id)
		.def_readonly("name", &VersionReport::name)
		.def_readonly("major", &VersionReport::major)
		.def_readonly("minor", &VersionReport::minor)
		.def_readonly("maintenance", &VersionReport::maintenance)
		.def_readonly("build", &VersionReport::build);
}

} // namespace icsneo
