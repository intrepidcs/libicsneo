#ifndef __RADA2B_H_
#define __RADA2B_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/rada2b/rada2bsettings.h"

namespace icsneo {

class RADA2B : public Device {
public:
	// Serial numbers start with AB
	// USB PID is 0x0006, standard driver is DXX
	// Ethernet MAC allocation is 0x18, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADA2B, DeviceType::RAD_A2B, "AB");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01,

			Network::NetID::A2B_01,
			Network::NetID::A2B_02,

			Network::NetID::I2C_01,
			Network::NetID::I2C_02,

			Network::NetID::MDIO_01,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADA2B;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADA2B_ZCHIP, true, "ZCHIP", "RADA2B_SW_bin", 0, FirmwareType::Zip},
			{ChipID::RADA2B_REVB_ZCHIP, false, "ZCHIP", "RADA2B_REVB_SW_bin", 0, FirmwareType::Zip}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADA2B_ZCHIP, BootloaderCommunication::RAD)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}
protected:
	RADA2B(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADA2BSettings, Disk::NeoMemoryDiskDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 15*1024*1024;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

};

}

#endif // __cplusplus

#endif