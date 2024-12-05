#ifndef __PLASION_H_
#define __PLASION_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/device/extensions/flexray/extension.h"

namespace icsneo {

class Plasion : public Device {
public:
	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_mscan,
			Network::_icsneo_netid_t::HSCAN2,
			Network::_icsneo_netid_t::HSCAN3,
			Network::_icsneo_netid_t::HSCAN4,
			Network::_icsneo_netid_t::HSCAN5,
			Network::_icsneo_netid_t::HSCAN6,
			Network::_icsneo_netid_t::HSCAN7,

			Network::_icsneo_netid_t::icsneo_netid_lsftcan,
			Network::_icsneo_netid_t::LSFTCAN2,

			Network::_icsneo_netid_t::icsneo_netid_swcan,
			Network::_icsneo_netid_t::SWCAN2,

			Network::_icsneo_netid_t::Ethernet,

			Network::_icsneo_netid_t::icsneo_netid_lin,
			Network::_icsneo_netid_t::LIN2,
			Network::_icsneo_netid_t::LIN3,
			Network::_icsneo_netid_t::LIN4,

			Network::_icsneo_netid_t::FlexRay
		};
		return supportedNetworks;
	}

	// Until VNET support is added, assume we have one FIRE 2 VNET or FlexRay VNETZ as the main
	size_t getEthernetActivationLineCount() const override { return 1; }

protected:
	using Device::Device;

	// TODO This is done so that Plasion can still transmit it's basic networks, awaiting slave VNET support
	virtual bool isSupportedRXNetwork(const Network&) const override { return true; }
	virtual bool isSupportedTXNetwork(const Network&) const override { return true; }
	virtual void setupExtensions() override {
		std::vector<Network> flexRayControllers;
		flexRayControllers.push_back(Network::_icsneo_netid_t::FlexRay);
		flexRayControllers.push_back(Network::_icsneo_netid_t::FlexRay); // TODO Becomes FlexRay2 if not in coldstart mode
		addExtension(std::make_shared<FlexRay::Extension>(*this, flexRayControllers));
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
		// TODO Check configuration for FlexRay ColdStart mode, disable FlexRay 2 if so
	}

	virtual std::vector<std::shared_ptr<FlexRay::Controller>> getFlexRayControllers() const override {
		// TODO Check configuration for FlexRay Enabled

		auto extension = getExtension<FlexRay::Extension>();
		if(!extension)
			return Device::getFlexRayControllers();
		
		std::vector<std::shared_ptr<FlexRay::Controller>> ret;

		if(auto ctrl1 = extension->getController(0))
			ret.push_back(std::move(ctrl1));

		// TODO Check configuration for FlexRay ColdStart mode, FlexRay2 -> FlexRay if so
		if(auto ctrl2 = extension->getController(1))
			ret.push_back(std::move(ctrl2));

		return ret;
	}

	void handleDeviceStatus(const std::shared_ptr<InternalMessage>& message) override {
		if(message->data.size() < sizeof(fire2vnet_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const fire2vnet_status_t* status = reinterpret_cast<const fire2vnet_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	bool supportsWiVI() const override { return true; }
};

}

#endif // __cplusplus

#endif