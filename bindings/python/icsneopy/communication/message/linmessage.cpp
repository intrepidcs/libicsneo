#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/linmessage.h"

namespace icsneo {

void init_linmessage(pybind11::module_& m) {
	pybind11::classh<LINErrorFlags>(m, "LINErrorFlags")
		.def_readwrite("ErrRxBreakOnly", &LINErrorFlags::ErrRxBreakOnly)
		.def_readwrite("ErrRxBreakSyncOnly", &LINErrorFlags::ErrRxBreakSyncOnly)
		.def_readwrite("ErrTxRxMismatch", &LINErrorFlags::ErrTxRxMismatch)
		.def_readwrite("ErrRxBreakNotZero", &LINErrorFlags::ErrRxBreakNotZero)
		.def_readwrite("ErrRxBreakTooShort", &LINErrorFlags::ErrRxBreakTooShort)
		.def_readwrite("ErrRxSyncNot55", &LINErrorFlags::ErrRxSyncNot55)
		.def_readwrite("ErrRxDataLenOver8", &LINErrorFlags::ErrRxDataLenOver8)
		.def_readwrite("ErrFrameSync", &LINErrorFlags::ErrFrameSync)
		.def_readwrite("ErrFrameMessageID", &LINErrorFlags::ErrFrameMessageID)
		.def_readwrite("ErrFrameResponderData", &LINErrorFlags::ErrFrameResponderData)
		.def_readwrite("ErrChecksumMatch", &LINErrorFlags::ErrChecksumMatch);

	pybind11::classh<LINStatusFlags>(m, "LINStatusFlags")
		.def_readwrite("TxChecksumEnhanced", &LINStatusFlags::TxChecksumEnhanced)
		.def_readwrite("TxCommander", &LINStatusFlags::TxCommander)
		.def_readwrite("TxResponder", &LINStatusFlags::TxResponder)
		.def_readwrite("TxAborted", &LINStatusFlags::TxAborted)
		.def_readwrite("UpdateResponderOnce", &LINStatusFlags::UpdateResponderOnce)
		.def_readwrite("HasUpdatedResponderOnce", &LINStatusFlags::HasUpdatedResponderOnce)
		.def_readwrite("BusRecovered", &LINStatusFlags::BusRecovered)
		.def_readwrite("BreakOnly", &LINStatusFlags::BreakOnly);

	pybind11::classh<LINMessage, Frame> linMessage(m, "LINMessage");

	pybind11::enum_<LINMessage::Type>(linMessage, "Type")
		.value("NOT_SET", LINMessage::Type::NOT_SET)
		.value("LIN_COMMANDER_MSG", LINMessage::Type::LIN_COMMANDER_MSG)
		.value("LIN_HEADER_ONLY", LINMessage::Type::LIN_HEADER_ONLY)
		.value("LIN_BREAK_ONLY", LINMessage::Type::LIN_BREAK_ONLY)
		.value("LIN_SYNC_ONLY", LINMessage::Type::LIN_SYNC_ONLY)
		.value("LIN_UPDATE_RESPONDER", LINMessage::Type::LIN_UPDATE_RESPONDER)
		.value("LIN_ERROR", LINMessage::Type::LIN_ERROR);

	linMessage
		.def(pybind11::init<>())
		.def(pybind11::init<uint8_t>())
		.def_static("calc_checksum", &LINMessage::calcChecksum)
		.def("calc_protected_id", &LINMessage::calcProtectedID)
		.def_readwrite("ID", &LINMessage::ID)
		.def_readwrite("protectedID", &LINMessage::protectedID)
		.def_readwrite("checksum", &LINMessage::checksum)
		.def_readwrite("linMsgType", &LINMessage::linMsgType)
		.def_readwrite("isEnhancedChecksum", &LINMessage::isEnhancedChecksum)
		.def_readwrite("errFlags", &LINMessage::errFlags)
		.def_readwrite("statusFlags", &LINMessage::statusFlags);
}

} // namespace icsneo

