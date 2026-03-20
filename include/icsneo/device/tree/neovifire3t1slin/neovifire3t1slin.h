#ifndef __NEOVIFIRE3T1SLIN_H_
#define __NEOVIFIRE3T1SLIN_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/neovifire3t1slin/neovifire3t1slinsettings.h"

namespace icsneo {

class NeoVIFIRE3T1SLIN : public Device {
public:
	// Serial numbers start with FT
	// Ethernet MAC allocation is 0x1E, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE3T1SLIN, DeviceType::FIRE3_T1S_LIN, "FT");

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

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04,
			Network::NetID::LIN_05,
			Network::NetID::LIN_06,
			Network::NetID::LIN_07,
			Network::NetID::LIN_08,
			Network::NetID::LIN_09,
			Network::NetID::LIN_10,
			
			Network::NetID::ISO9141_01,
			Network::NetID::ISO9141_02,
			Network::NetID::ISO9141_03,
			Network::NetID::ISO9141_04,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,

			Network::NetID::AE_01,
			Network::NetID::AE_02,
			Network::NetID::AE_03,
			Network::NetID::AE_04,
			Network::NetID::AE_05,
			Network::NetID::AE_06,
			Network::NetID::AE_07,
			Network::NetID::AE_08,
		};
		return supportedNetworks;
	}

	bool supportsTC10() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::neoVIFIRE3;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::neoVIFIRE3_ZCHIP, true, "ZCHIP", "fire3_zchip_ief", 0, FirmwareType::IEF},
			{ChipID::neoVIFIRE3_SCHIP, true, "SCHIP", "fire3_schip_ief", 1, FirmwareType::IEF},
			{ChipID::neoVIFIRE3_LINUX, true, "Linux Flash", "fire3_lnx_flash_ief", 2, FirmwareType::IEF},
			{ChipID::VEM_04_T1S_LIN_ZCHIP, true, "VEM-04-Z", "vem_04_t1s_lin_zchip_ief", 3, FirmwareType::IEF},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<FlashPhase>(ChipID::neoVIFIRE3_ZCHIP, BootloaderCommunication::Application, true, false)
			.add<FlashPhase>(ChipID::neoVIFIRE3_SCHIP, BootloaderCommunication::Application, false, true)
			.add<FlashPhase>(ChipID::neoVIFIRE3_LINUX, BootloaderCommunication::Application, false, false, false)
			.add<FlashPhase>(ChipID::VEM_04_T1S_LIN_ZCHIP, BootloaderCommunication::Application, false, false)
			.add<FinalizePhase>(ChipID::neoVIFIRE3_ZCHIP, BootloaderCommunication::Application)
			.add<FinalizePhase>(ChipID::neoVIFIRE3_SCHIP, BootloaderCommunication::Application)
			.add<FinalizePhase>(ChipID::neoVIFIRE3_LINUX, BootloaderCommunication::Application)
			.add<FinalizePhase>(ChipID::VEM_04_T1S_LIN_ZCHIP, BootloaderCommunication::Application)
			.add<EnterApplicationPhase>(ChipID::neoVIFIRE3_ZCHIP)
			.add<ReconnectPhase>()
			.addSetting(BootloaderSetting::UpdateAll, true);
	}
protected:
	NeoVIFIRE3T1SLIN(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRE3T1SLINSettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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
		return std::nullopt;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
	
	bool supportsEraseMemory() const override {
		return true;
	}

	size_t getDiskCount() const override { 
		return 2;
	}

	bool supportsNetworkMutex() const override { 
		return true; 
	}

};

}

#endif