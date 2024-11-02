#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/message.h"

namespace icsneo {

void init_message(pybind11::module_& m) {
	pybind11::class_<Message, std::shared_ptr<Message>> message(m, "Message");
	pybind11::enum_<Message::Type>(message, "Type")
		.value("Frame", Message::Type::Frame)
		.value("CANErrorCount", Message::Type::CANErrorCount)
		.value("LINHeaderOnly", Message::Type::LINHeaderOnly)
		.value("LINBreak", Message::Type::LINBreak)
		.value("Invalid", Message::Type::Invalid)
		.value("RawMessage", Message::Type::RawMessage)
		.value("ReadSettings", Message::Type::ReadSettings)
		.value("ResetStatus", Message::Type::ResetStatus)
		.value("DeviceVersion", Message::Type::DeviceVersion)
		.value("Main51", Message::Type::Main51)
		.value("FlexRayControl", Message::Type::FlexRayControl)
		.value("EthernetPhyRegister", Message::Type::EthernetPhyRegister)
		.value("LogicalDiskInfo", Message::Type::LogicalDiskInfo)
		.value("ExtendedResponse", Message::Type::ExtendedResponse)
		.value("WiVICommandResponse", Message::Type::WiVICommandResponse)
		.value("ScriptStatus", Message::Type::ScriptStatus)
		.value("ComponentVersions", Message::Type::ComponentVersions)
		.value("SupportedFeatures", Message::Type::SupportedFeatures)
		.value("GenericBinaryStatus", Message::Type::GenericBinaryStatus)
		.value("LiveData", Message::Type::LiveData)
		.value("HardwareInfo", Message::Type::HardwareInfo)
		.value("TC10Status", Message::Type::TC10Status)
		.value("AppError", Message::Type::AppError);

	message.def(pybind11::init<Message::Type>());
	message.def_readonly("type", &Message::type);
	message.def_readwrite("timestamp", &Message::timestamp);

	pybind11::class_<RawMessage, std::shared_ptr<RawMessage>, Message>(m, "RawMessage")
		.def_readwrite("network", &RawMessage::network)
		.def_readwrite("data", &RawMessage::data);

	pybind11::class_<Frame, std::shared_ptr<Frame>, RawMessage>(m, "Frame")
		.def_readwrite("description", &Frame::description)
		.def_readwrite("transmitted", &Frame::transmitted)
		.def_readwrite("error", &Frame::error);
}

} // namespace icsneo

