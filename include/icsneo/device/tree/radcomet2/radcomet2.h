#ifndef __RADCOMET2_H_
#define __RADCOMET2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radcomet2/radcomet2settings.h"

namespace icsneo {

class RADComet2 : public Device {
public:

	// Serial numbers start with RC
	// USB PID is 0x1207, standard driver is DXX
	// Ethernet MAC allocation is 0x1D, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE_BY_SERIAL_RANGE(RADComet2, DeviceType::RADComet2, "RC0300", "RCZZZZ");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::AE_01,
			Network::NetID::AE_02,
			Network::NetID::AE_03,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
			Network::NetID::MDIO_03,
			Network::NetID::MDIO_04,

			Network::NetID::LIN_01,
			Network::NetID::ISO9141_01,
		};
		return supportedNetworks;
	}

	std::string getProductName() const override {
		return "RAD-Comet 2";
	}

	bool getEthPhyRegControlSupported() const override { return true; }
	bool supportsTC10() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADComet2;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADComet2_ZYNQ, true, "ZCHIP", "RADComet_SW_bin", 0, FirmwareType::Zip},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADComet2_ZYNQ, BootloaderCommunication::RAD)
			.add<EnterApplicationPhase>(ChipID::RADComet2_ZYNQ)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}

protected:
	RADComet2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADComet2Settings>(makeDriver);
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
		decoder.timestampResolution = 10;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 32*1024*1024;
	}

};

}

#endif // __cplusplus

#endif
