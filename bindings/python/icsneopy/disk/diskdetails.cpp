#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "icsneo/disk/diskdetails.h"

namespace icsneo {

void init_diskdetails(pybind11::module_& m) {
	pybind11::enum_<DiskLayout>(m, "DiskLayout")
		.value("Spanned", DiskLayout::Spanned)
		.value("RAID0", DiskLayout::RAID0);

	pybind11::classh<DiskInfo>(m, "DiskInfo")
		.def(pybind11::init())
		.def_readwrite("present", &DiskInfo::present)
		.def_readwrite("initialized", &DiskInfo::initialized)
		.def_readwrite("formatted", &DiskInfo::formatted)
		.def_readwrite("sectors", &DiskInfo::sectors)
		.def_readwrite("bytes_per_sector", &DiskInfo::bytesPerSector)
		.def("size", &DiskInfo::size);

	pybind11::classh<DiskDetails>(m, "DiskDetails")
		.def(pybind11::init())
		.def_readwrite("layout", &DiskDetails::layout)
		.def_readwrite("full_format", &DiskDetails::fullFormat)
		.def_readwrite("disks", &DiskDetails::disks);
}

} // namespace icsneo
