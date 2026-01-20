#ifndef __RADCOMETBASE_H_
#define __RADCOMETBASE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class RADCometBase : public Device {
public:
	virtual const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::AE_01,
			Network::NetID::AE_02,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
			Network::NetID::MDIO_03,
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADComet;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADComet_ZYNQ, true, "ZCHIP", "RADComet_SW_bin", 0, FirmwareType::Zip},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override { 
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADComet_ZYNQ, BootloaderCommunication::RAD)
			.add<EnterApplicationPhase>(ChipID::RADComet_ZYNQ)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}

protected:
	using Device::Device;

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