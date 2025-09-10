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
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,
			Network::NetID::DWCAN_06,
			Network::NetID::DWCAN_07,

			Network::NetID::LSFTCAN_01,
			Network::NetID::LSFTCAN_02,

			Network::NetID::SWCAN_01,
			Network::NetID::SWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04,

			Network::NetID::FLEXRAY_01
		};
		return supportedNetworks;
	}

	// Until VNET support is added, assume we have one FIRE 2 VNET or FlexRay VNETZ as the main
	size_t getEthernetActivationLineCount() const override { return 1; }

	bool supportsComponentVersions() const override { return true; }

protected:
	using Device::Device;

	// TODO This is done so that Plasion can still transmit it's basic networks, awaiting slave VNET support
	virtual bool isSupportedRXNetwork(const Network&) const override { return true; }
	virtual bool isSupportedTXNetwork(const Network&) const override { return true; }
	virtual void setupExtensions() override {
		std::vector<Network> flexRayControllers;
		flexRayControllers.push_back(Network::NetID::FLEXRAY_01);
		flexRayControllers.push_back(Network::NetID::FLEXRAY_01); // TODO Becomes FlexRay2 if not in coldstart mode
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

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
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