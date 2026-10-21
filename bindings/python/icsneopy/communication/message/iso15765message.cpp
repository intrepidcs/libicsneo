#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "icsneo/communication/message/iso15765message.h"

namespace icsneo {

void init_iso15765message(pybind11::module_& m) {
	auto args = pybind11::classh<Iso15765MessageArgs>(m, "Iso15765MessageArgs");

	pybind11::classh<Iso15765MessageArgs::ArbId>(args, "ArbId")
		.def(pybind11::init<uint32_t, bool>(), pybind11::arg("id") = 0, pybind11::arg("is_29bit") = false)
		.def_readwrite("id", &Iso15765MessageArgs::ArbId::Id)
		.def_readwrite("is_29bit", &Iso15765MessageArgs::ArbId::Is29Bit);

	args.def(pybind11::init<>())
		.def_property("tx_index", &Iso15765MessageArgs::getTxIndex, &Iso15765MessageArgs::setTxIndex)
		.def_property("arb_id",
			[](Iso15765MessageArgs& self) -> Iso15765MessageArgs::ArbId& {
				return const_cast<Iso15765MessageArgs::ArbId&>(self.getArbId());
			},
			&Iso15765MessageArgs::setArbId,
			pybind11::return_value_policy::reference_internal)
		.def_property("flow_control_arb_id",
			[](Iso15765MessageArgs& self) -> Iso15765MessageArgs::ArbId& {
				return const_cast<Iso15765MessageArgs::ArbId&>(self.getFlowControlArbId());
			},
			&Iso15765MessageArgs::setFlowControlArbId,
			pybind11::return_value_policy::reference_internal)
		.def_property("flow_control_arb_id_mask", &Iso15765MessageArgs::getFlowControlArbIdMask, &Iso15765MessageArgs::setFlowControlArbIdMask)
		.def_property("is_can_fd", &Iso15765MessageArgs::getIsCanFd, &Iso15765MessageArgs::setIsCanFd)
		.def_property("is_brs_enabled", &Iso15765MessageArgs::getIsBrsEnabled, &Iso15765MessageArgs::setIsBrsEnabled)
		.def_property("tx_dl", &Iso15765MessageArgs::getTxDl, &Iso15765MessageArgs::setTxDl)
		.def_property("st_min", &Iso15765MessageArgs::getStMin, &Iso15765MessageArgs::setStMin)
		.def_property("block_size", &Iso15765MessageArgs::getBlockSize, &Iso15765MessageArgs::setBlockSize)
		.def_property("extended_address", &Iso15765MessageArgs::getExtendedAddress, &Iso15765MessageArgs::setExtendedAddress)
		.def_property("flow_control_extended_address", &Iso15765MessageArgs::getFlowControlExtendedAddress, &Iso15765MessageArgs::setFlowControlExtendedAddress)
		.def_property("padding_value", &Iso15765MessageArgs::getPaddingValue, &Iso15765MessageArgs::setPaddingValue)
		.def_property("fs_timeout", &Iso15765MessageArgs::getFsTimeout, &Iso15765MessageArgs::setFsTimeout)
		.def_property("fs_wait_timeout", &Iso15765MessageArgs::getFsWaitTimeout, &Iso15765MessageArgs::setFsWaitTimeout)
		.def_property("cf_timeout", &Iso15765MessageArgs::getCfTimeout, &Iso15765MessageArgs::setCfTimeout)
		.def_property("data",
			&Iso15765MessageArgs::getData,
			[](Iso15765MessageArgs& self, std::vector<uint8_t> data) { self.setData(std::move(data)); })
		.def_property("is_flow_control_enabled", &Iso15765MessageArgs::getIsFlowControlEnabled, &Iso15765MessageArgs::setIsFlowControlEnabled);
}

} // namespace icsneo
