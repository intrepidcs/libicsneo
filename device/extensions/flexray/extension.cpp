#include "icsneo/device/extensions/flexray/extension.h"
#include "icsneo/device/device.h"

using namespace icsneo;

FlexRay::Extension::Extension(Device& device, uint8_t controllerCount) : DeviceExtension(device) {
	for(uint8_t i = 0; i < controllerCount; i++)
		controllers.emplace_back(std::make_shared<FlexRay::Controller>(device, i));
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