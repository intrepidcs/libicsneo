#ifndef __NEOVIFIRE3_H_
#define __NEOVIFIRE3_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/neovifire3/neovifire3settings.h"

namespace icsneo {

class NeoVIFIRE3 : public Device {
public:
	// Serial numbers start with ON
	// Ethernet MAC allocation is 0x0E, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE3, _icsneo_devicetype_t::icsneo_devicetype_fire3, "ON");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			_icsneo_netid_t::icsneo_netid_hscan,
			_icsneo_netid_t::icsneo_netid_mscan,
			_icsneo_netid_t::icsneo_netid_hscan2,
			_icsneo_netid_t::icsneo_netid_hscan3,
			_icsneo_netid_t::icsneo_netid_hscan4,
			_icsneo_netid_t::icsneo_netid_hscan5,
			_icsneo_netid_t::icsneo_netid_hscan6,
			_icsneo_netid_t::icsneo_netid_hscan7,
			_icsneo_netid_t::icsneo_netid_dwcan9,
			_icsneo_netid_t::icsneo_netid_dwcan10,
			_icsneo_netid_t::icsneo_netid_dwcan11,
			_icsneo_netid_t::icsneo_netid_dwcan12,
			_icsneo_netid_t::icsneo_netid_dwcan13,
			_icsneo_netid_t::icsneo_netid_dwcan14,
			_icsneo_netid_t::icsneo_netid_dwcan15,
			_icsneo_netid_t::icsneo_netid_dwcan16,

			_icsneo_netid_t::icsneo_netid_ethernet,
			_icsneo_netid_t::icsneo_netid_ethernet2,
			_icsneo_netid_t::icsneo_netid_ethernet3,

			_icsneo_netid_t::icsneo_netid_lin,
			_icsneo_netid_t::icsneo_netid_lin2,
			_icsneo_netid_t::icsneo_netid_lin3,
			_icsneo_netid_t::icsneo_netid_lin4,
			_icsneo_netid_t::icsneo_netid_lin5,
			_icsneo_netid_t::icsneo_netid_lin6,
			_icsneo_netid_t::icsneo_netid_lin7,
			_icsneo_netid_t::icsneo_netid_lin8,
		};
		return supportedNetworks;
	}

	bool supportsComponentVersions() const override { return true; }

protected:
	NeoVIFIRE3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRE3Settings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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