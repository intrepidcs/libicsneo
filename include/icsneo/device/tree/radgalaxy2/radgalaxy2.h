#ifndef __RADGALAXY2_H_
#define __RADGALAXY2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgalaxy2/radgalaxy2settings.h"

namespace icsneo {

class RADGalaxy2 : public Device {
public:
	// Serial numbers start with G2
	// Ethernet MAC allocation is 0x17, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGalaxy2, DeviceType::RADGalaxy2, "G2");

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

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,
			Network::NetID::ETHERNET_03,

			Network::NetID::AE_01,
			Network::NetID::AE_02,
			Network::NetID::AE_03,
			Network::NetID::AE_04,
			Network::NetID::AE_05,
			Network::NetID::AE_06,
			Network::NetID::AE_07,
			Network::NetID::AE_08,
			Network::NetID::AE_09,
			Network::NetID::AE_10,
			Network::NetID::AE_11,
			Network::NetID::AE_12,

			Network::NetID::ISO9141_01,
			Network::NetID::ISO9141_02,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
			Network::NetID::MDIO_03,
			Network::NetID::MDIO_04,
			Network::NetID::MDIO_05,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool supportsTC10() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADGalaxy2;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RAD_GALAXY_2_ZMPCHIP_ID, true, "ZCHIP", "RADGalaxy2_SW_bin_p1", {"RADGalaxy2_SW_bin_p1",  "RADGalaxy2_SW_bin_p2"}, 0, FirmwareType::Zip},
			{ChipID::RADGALAXY2_SYSMON_CHIP, true, "MCHIP", "galaxy2_sysmon_ief", 1, FirmwareType::IEF}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RAD_GALAXY_2_ZMPCHIP_ID, BootloaderCommunication::RAD)
			.add<ReconnectPhase>()
			.add<FlashPhase>(ChipID::RADGALAXY2_SYSMON_CHIP, BootloaderCommunication::RADGalaxy2Peripheral)
			.add<EnterApplicationPhase>(ChipID::RAD_GALAXY_2_ZMPCHIP_ID)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}

protected:
	RADGalaxy2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGalaxy2Settings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
		encoder.supportEthPhy = true;
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
		if(message->data.size() < sizeof(radgalaxy2_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgalaxy2_status_t* status = reinterpret_cast<const radgalaxy2_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif // __cplusplus

#endif