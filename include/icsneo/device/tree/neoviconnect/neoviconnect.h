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
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,
			Network::NetID::DWCAN_06,
			Network::NetID::DWCAN_07,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02
		};
		return supportedNetworks;
	}

	ProductID getProductID() const override {
		return ProductID::Connect;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::Connect_ZCHIP, true, "ZCHIP", "neovi_connect_zchip_ief", 0, FirmwareType::IEF},
			{ChipID::Connect_LINUX, true, "Linux Flash", "neovi_connect_lnx_flash_ief", 1, FirmwareType::IEF},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<FlashPhase>(ChipID::Connect_ZCHIP, BootloaderCommunication::Application, true, false, false)
			.add<FlashPhase>(ChipID::Connect_LINUX, BootloaderCommunication::Application, false, false, false)
			.add<FinalizePhase>(ChipID::Connect_ZCHIP, BootloaderCommunication::Application)
			.add<FinalizePhase>(ChipID::Connect_LINUX, BootloaderCommunication::Application)
			.add<ReconnectPhase>()
			.addSetting(BootloaderSetting::UpdateAll, true);
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

	bool supportsGPTP() const override { return true; }
	
	size_t getDiskCount() const override {
		return 2;
	}

};

}

#endif