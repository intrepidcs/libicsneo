#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/api/version.h"

#include <sstream>

namespace icsneo {

void init_version(pybind11::module_& m) {
	pybind11::classh<neoversion_t>(m, "NeoVersion")
		.def_readonly("major", &neoversion_t::major)
		.def_readonly("minor", &neoversion_t::minor)
		.def_readonly("patch", &neoversion_t::patch)
		.def_readonly("metadata", &neoversion_t::metadata)
		.def_readonly("buildBranch", &neoversion_t::buildBranch)
		.def_readonly("buildTag", &neoversion_t::buildTag)
		.def("__repr__", [](const neoversion_t& self) -> std::string {
			std::stringstream ss;
			ss << self;
			return ss.str();
		});
	m.def("get_version", &GetVersion);
}

} // namespace icsneo 

