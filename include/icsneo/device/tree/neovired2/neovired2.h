#ifndef __NEOVIRED2_H_
#define __NEOVIRED2_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/neovired2/neovired2settings.h"

namespace icsneo {

class NeoVIRED2 : public Device {
public:
	// Serial numbers start with D2
	// Ethernet MAC allocation is 0x0E, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIRED2, _icsneo_devicetype_t::icsneo_devicetype_red2, "D2");

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

			Network::_icsneo_netid_t::Ethernet,
			Network::_icsneo_netid_t::Ethernet2,

			Network::_icsneo_netid_t::icsneo_netid_lin,
			Network::_icsneo_netid_t::LIN2
		};
		return supportedNetworks;
	}

	bool supportsComponentVersions() const override { return true; }

protected:
	NeoVIRED2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIRED2Settings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif