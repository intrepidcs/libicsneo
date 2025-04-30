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
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE3, DeviceType::FIRE3, "ON");

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
			Network::NetID::DWCAN_09,
			Network::NetID::DWCAN_10,
			Network::NetID::DWCAN_11,
			Network::NetID::DWCAN_12,
			Network::NetID::DWCAN_13,
			Network::NetID::DWCAN_14,
			Network::NetID::DWCAN_15,
			Network::NetID::DWCAN_16,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,
			Network::NetID::ETHERNET_03,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04,
			Network::NetID::LIN_05,
			Network::NetID::LIN_06,
			Network::NetID::LIN_07,
			Network::NetID::LIN_08,
		};
		return supportedNetworks;
	}
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

	bool supportsGPTP() const override { return true; }

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