#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/callback/messagecallback.h"

namespace icsneo {

void init_messagecallback(pybind11::module_& m) {
	pybind11::class_<MessageCallback, std::shared_ptr<MessageCallback>>(m, "MessageCallback")
		.def(pybind11::init<MessageCallback::fn_messageCallback, std::shared_ptr<MessageFilter>>());
}

} // namespace icsneo 

