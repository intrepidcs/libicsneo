#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/device/extensions/bootloader/api.h"
#include "icsneo/device/extensions/bootloader/parser/fwloader.h"

namespace icsneo {

struct BootloaderNamespace {};

void init_bootloader(pybind11::module_& m) {
	// Dummy class to act as Bootloader namespace
	pybind11::classh<BootloaderNamespace> bootloader(m, "Bootloader");

	// Opaque handle returned by FWLoader.load and consumed by update
	pybind11::classh<Bootloader::FWRepo>(bootloader, "FWRepo");

	pybind11::classh<Bootloader::FWLoader>(bootloader, "FWLoader")
		.def_static("load", &Bootloader::FWLoader::Load, pybind11::arg("path"), pybind11::call_guard<pybind11::gil_scoped_release>());

	bootloader.def_static("update",
		[](Device& device, const Bootloader::FWRepo& fw, Bootloader::Logger logger, bool alreadyInBootloader, bool forceFlash) {
			if(!logger) {
				logger = [](Device::OpenStatusType, const std::string&, std::optional<double>) { return Device::OpenDirective::Continue; };
			}
			return Bootloader::update(device, fw, logger, alreadyInBootloader, forceFlash);
		},
		pybind11::arg("device"),
		pybind11::arg("fw"),
		pybind11::arg("logger") = pybind11::none(),
		pybind11::arg("already_in_bootloader") = false,
		pybind11::arg("force_flash") = false,
		pybind11::call_guard<pybind11::gil_scoped_release>());
}

} // namespace icsneo
