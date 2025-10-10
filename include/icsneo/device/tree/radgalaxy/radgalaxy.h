#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgalaxy/radgalaxysettings.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	// Ethernet MAC allocation is 0x03, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGalaxy, DeviceType::RADGalaxy, "RG");

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

			Network::NetID::SWCAN_01,
			Network::NetID::SWCAN_02,

			Network::NetID::LIN_01,

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
			Network::NetID::AE_09,
			Network::NetID::AE_10,
			Network::NetID::AE_11,
			Network::NetID::AE_12,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
			Network::NetID::MDIO_03,
			Network::NetID::MDIO_04,
			Network::NetID::MDIO_05,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }
	bool supportsGPTP() const override { return true; }


	ProductID getProductID() const override {
		return ProductID::RADGalaxy;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADGalaxy_ZYNQ, true, "ZCHIP", "RADGalaxy_SG2_SW_bin", 0, FirmwareType::Zip},
			{ChipID::RADGalaxy_FFG_Zynq, false, "ZCHIP", "RADGalaxy_SG2_FFG_SW_bin", 0, FirmwareType::Zip}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADGalaxy_ZYNQ, BootloaderCommunication::RAD)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}
protected:
	RADGalaxy(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGalaxySettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
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
		if(message->data.size() < sizeof(radgalaxy_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgalaxy_status_t* status = reinterpret_cast<const radgalaxy_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	size_t getDiskCount() const override {
		return 1;
	}
};

}

#endif // __cplusplus

#endif