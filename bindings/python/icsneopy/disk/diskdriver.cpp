#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/disk/diskdriver.h"

namespace icsneo {

// binding namespace workaround to avoid submodules
struct DiskNamespace {
	using Access = icsneo::Disk::Access;
	using MemoryType = icsneo::Disk::MemoryType;
};

void init_diskdriver(pybind11::module_& m) {
	pybind11::class_<DiskNamespace> disk(m, "Disk");
	pybind11::enum_<Disk::Access>(disk, "Access")
		.value("None", Disk::Access::None)
		.value("EntireCard", Disk::Access::EntireCard)
		.value("VSA", Disk::Access::VSA);
	pybind11::enum_<Disk::MemoryType>(disk, "MemoryType")
		.value("Flash", Disk::MemoryType::Flash)
		.value("SD", Disk::MemoryType::SD);
}

} // namespace icsneo 
