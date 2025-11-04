#ifndef __RADCOMET3_H_
#define __RADCOMET3_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radcomet3/radcomet3.h"
#include "icsneo/device/tree/radcomet3/radcomet3settings.h"

namespace icsneo {

class RADComet3 : public Device {
public:

	// Serial numbers start with C3
	// USB PID is 0x1208, standard driver is DXX
	// Ethernet MAC allocation is 0x20, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADComet3, DeviceType::RADComet3, "C3");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::AE_01,
			Network::NetID::AE_02,
			Network::NetID::AE_03,
			Network::NetID::AE_04,
			Network::NetID::AE_05,
			Network::NetID::AE_06,
			Network::NetID::AE_07,

			Network::NetID::LIN_01,
			Network::NetID::ISO9141_01,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,

			Network::NetID::SPI_01,
			Network::NetID::SPI_02,
			Network::NetID::SPI_03,
			Network::NetID::SPI_04,
			Network::NetID::SPI_05,
			Network::NetID::SPI_06,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsTC10() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADComet3;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADCOMET3_ZCHIP, true, "ZCHIP", "RADComet3_SW_bin", 0, FirmwareType::Zip},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADCOMET3_ZCHIP, BootloaderCommunication::RAD)
			.add<WaitPhase>(std::chrono::milliseconds(5000))
			.add<ReconnectPhase>();
	}

protected:
	RADComet3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADComet3Settings>(makeDriver);
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

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 32*1024*1024;
	}

};

}

#endif // __cplusplus

#endif
