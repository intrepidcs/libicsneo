#include "icsneo/device/extensions/flexray/extension.h"
#include "icsneo/device/device.h"
#include "icsneo/communication/message/flexray/flexraymessage.h"

using namespace icsneo;

FlexRay::Extension::Extension(Device& device, const std::vector<Network>& controllerNetworks) : DeviceExtension(device) {
	for(uint8_t i = 0; i < controllerNetworks.size(); i++)
		controllers.emplace_back(std::make_shared<FlexRay::Controller>(device, i, controllerNetworks[i]));
}

void FlexRay::Extension::onGoOnline() {
	for(auto& controller : controllers) {
		if(controller->getStartWhenGoingOnline())
			controller->getReady();
	}
	for(auto& controller : controllers) {
		if(controller->getStartWhenGoingOnline())
			controller->start();
	}
}

void FlexRay::Extension::onGoOffline() {
	for(auto& controller : controllers)
		controller->halt();
}

void FlexRay::Extension::handleMessage(const std::shared_ptr<Message>& message) {
	switch(message->network.getNetID()) {
		case Network::NetID::FlexRayControl: {
			auto msg = std::dynamic_pointer_cast<FlexRayControlMessage>(message);
			if(!msg || !msg->decoded)
				return;
			switch(msg->opcode) {
				case FlexRay::Opcode::ReadCCStatus:
					if(auto status = std::dynamic_pointer_cast<FlexRayControlMessage>(message)) { // TODO else report error?
						if(status->controller >= controllers.size())
							return; // TODO error
						controllers[status->controller]->_setStatus(status);
					}
					break;
			}
			break;
		}
		default:
			break;
	}
}

bool FlexRay::Extension::transmitHook(const std::shared_ptr<Message>& message, bool& success) {
	if(!message || message->network.getType() != Network::Type::FlexRay)
		return true; // Don't hook non-FlexRay messages

	success = false;

	std::shared_ptr<FlexRayMessage> frmsg = std::dynamic_pointer_cast<FlexRayMessage>(message);
	if(!frmsg)
		return false;

	for(auto& controller : controllers) {
		if(controller->getNetwork() != message->network)
			continue;
		success |= controller->transmit(frmsg);
	}

	return false;
}