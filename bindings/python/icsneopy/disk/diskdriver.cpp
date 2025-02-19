#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/disk/diskdriver.h"

namespace icsneo {

void init_diskdriver(pybind11::module_& m) {
	auto disk = m.def_submodule("disk");
	pybind11::enum_<Disk::Access>(disk, "Access")
		.value("None", Disk::Access::None)
		.value("EntireCard", Disk::Access::EntireCard)
		.value("VSA", Disk::Access::VSA);
	pybind11::enum_<Disk::MemoryType>(disk, "MemoryType")
		.value("Flash", Disk::MemoryType::Flash)
		.value("SD", Disk::MemoryType::SD);
}

} // namespace icsneo 
