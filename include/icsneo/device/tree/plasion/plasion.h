#ifndef __PLASION_H_
#define __PLASION_H_

#include "icsneo/device/device.h"
#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/extensions/flexray/extension.h"

namespace icsneo {

class Plasion : public Device {
protected:
	virtual std::shared_ptr<Communication> makeCommunication(
		std::unique_ptr<ICommunication> transport,
		std::shared_ptr<Packetizer> packetizer,
		std::unique_ptr<Encoder> encoder,
		std::unique_ptr<Decoder> decoder
	) override { return std::make_shared<MultiChannelCommunication>(report, std::move(transport), packetizer, std::move(encoder), std::move(decoder)); }

	// TODO: This is done so that Plasion can still transmit it's basic networks, awaiting VLAN support
	virtual bool isSupportedRXNetwork(const Network&) const override { return true; }
	virtual bool isSupportedTXNetwork(const Network&) const override { return true; }
	virtual void setupExtensions() override {
		addExtension(std::make_shared<FlexRay::Extension>(*this, (uint8_t)2));
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::MSCAN,
			Network::NetID::HSCAN2,
			Network::NetID::HSCAN3,
			Network::NetID::HSCAN4,
			Network::NetID::HSCAN5,
			Network::NetID::HSCAN6,
			Network::NetID::HSCAN7,

			Network::NetID::LSFTCAN,
			Network::NetID::LSFTCAN2,

			Network::NetID::SWCAN,
			Network::NetID::SWCAN2,

			Network::NetID::Ethernet,

			Network::NetID::LIN,
			Network::NetID::LIN2,
			Network::NetID::LIN3,
			Network::NetID::LIN4,

			Network::NetID::FlexRay,
			Network::NetID::FlexRay2
		};
		return supportedNetworks;
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	virtual std::shared_ptr<FlexRay::Controller> getFlexRayControllerByNetwork(const Network& net) const override {
		uint8_t idx = 0xff;
		switch(net.getNetID()) {
			case Network::NetID::FlexRay:
				idx = 0;
				break;
			case Network::NetID::FlexRay2:
				idx = 1;
				break;
			default:
				return Device::getFlexRayControllerByNetwork(net);
		}
		
		auto extension = getExtension<FlexRay::Extension>();
		if(!extension)
			return Device::getFlexRayControllerByNetwork(net);
		
		auto res = extension->getController(idx);
		if(!res)
			return Device::getFlexRayControllerByNetwork(net);
		return res;
	}

public:
	Plasion(neodevice_t neodevice) : Device(neodevice) {}
};

}

#endif