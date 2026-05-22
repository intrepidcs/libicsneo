#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "icsneo/communication/message/allmacaddressesmessage.h"

namespace icsneo {

void init_allmacaddressesmessage(pybind11::module_& m) {
	pybind11::classh<AllMACAddressesMessage, Message>(m, "AllMACAddressesMessage")
		.def_readonly("addresses", &AllMACAddressesMessage::addresses);
}

} // namespace icsneo
