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
	ICSNEO_FINDABLE_DEVICE(NeoVIConnect, _icsneo_devicetype_t::icsneo_devicetype_connect, "DM");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_mscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,
			Network::_icsneo_netid_t::icsneo_netid_hscan3,
			Network::_icsneo_netid_t::icsneo_netid_hscan4,
			Network::_icsneo_netid_t::icsneo_netid_hscan5,
			Network::_icsneo_netid_t::HSCAN6,
			Network::_icsneo_netid_t::HSCAN7,

			Network::_icsneo_netid_t::Ethernet,

			Network::_icsneo_netid_t::icsneo_netid_lin,
			Network::_icsneo_netid_t::icsneo_netid_lin2
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
};

}

#endif