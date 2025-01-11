#ifndef __NEOVICONNECT_H_
#define __NEOVICONNECT_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/neoviconnect/neoviconnectsettings.h"

namespace icsneo {

class NeoVIConnect : public Device {
public:
	// Serial numbers start with DM
	// Ethernet MAC allocation is 0x1F, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIConnect, DeviceType::Connect, "DM");

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

			Network::NetID::Ethernet,

			Network::NetID::LIN,
			Network::NetID::LIN2
		};
		return supportedNetworks;
	}

protected:
	NeoVIConnect(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIConnectSettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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
		return 33*1024*1024;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsComponentVersions() const override { return true; }
	bool supportsGPTP() const override { return true; }

};

}

#endif