#ifndef __RADMOONT1S_H_
#define __RADMOONT1S_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/tree/radmoont1s/radmoont1ssettings.h"

namespace icsneo {

class RADMoonT1S : public Device {
public:

	// Serial numbers start with MS
	// USB PID is 0x1209, standard driver is DXX
	// Ethernet MAC allocation is 0x21, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADMoonT1S, DeviceType::RADMoonT1S, "MS");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::ETHERNET_01,

			Network::NetID::AE_01,

			Network::NetID::MDIO_01,

			Network::NetID::SPI_01,
		};
		return supportedNetworks;
	}


	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsTC10() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADMoonT1S;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADMOONT1S_ZCHIP, true, "ZCHIP", "RADMoonT1S_SW_bin", 1, FirmwareType::Zip}
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADMOONT1S_ZCHIP, BootloaderCommunication::RAD)
			.add<EnterApplicationPhase>(ChipID::RADMOONT1S_ZCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}
protected:
	RADMoonT1S(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoonT1SSettings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
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
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override {
		setupSupportedRXNetworks(txNetworks);
	}
};

}

#endif // __cplusplus

#endif // __RADMOONT1S_H_
