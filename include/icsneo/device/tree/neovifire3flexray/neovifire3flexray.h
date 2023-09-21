#ifndef __NEOVIFIRE3FLEXRAY_H_
#define __NEOVIFIRE3FLEXRAY_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/neovifire3flexray/neovifire3flexraysettings.h"

namespace icsneo {

class NeoVIFIRE3FlexRay : public Device {
public:
	// Serial numbers start with FF
	// Ethernet MAC allocation is 1F, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE3FlexRay, DeviceType::FIRE3_FlexRay, "FF");

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
			Network::NetID::DWCAN9,
			Network::NetID::DWCAN10,
			Network::NetID::DWCAN11,
			Network::NetID::DWCAN12,
			Network::NetID::DWCAN13,
			Network::NetID::DWCAN14,
			Network::NetID::DWCAN15,

			Network::NetID::Ethernet,
			Network::NetID::Ethernet2,
			Network::NetID::Ethernet3,

			Network::NetID::LIN,
			Network::NetID::LIN2,
			Network::NetID::LIN3,
			Network::NetID::LIN4,

			Network::NetID::FlexRay,
			Network::NetID::FlexRay1a,
			Network::NetID::FlexRay1b,
			Network::NetID::FlexRay2,
			Network::NetID::FlexRay2a,
			Network::NetID::FlexRay2b,
		};
		return supportedNetworks;
	}

protected:
	NeoVIFIRE3FlexRay(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRE3FlexRaySettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.align16bit = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool supportsWiVI() const override { return true; }
	
	bool supportsLiveData() const override { return true; }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif