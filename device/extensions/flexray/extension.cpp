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

bool FlexRay::Extension::transmitHook(const std::shared_ptr<Frame>& frame, bool& success) {
	if(!frame || frame->network.getType() != Network::Type::FlexRay)
		return true; // Don't hook non-FlexRay messages

	success = false;

	std::shared_ptr<FlexRayMessage> frmsg = std::dynamic_pointer_cast<FlexRayMessage>(frame);
	if(!frmsg)
		return false;

	for(auto& controller : controllers) {
		if(controller->getNetwork() != frame->network)
			continue;
		success |= controller->transmit(frmsg);
	}

	return false;
}