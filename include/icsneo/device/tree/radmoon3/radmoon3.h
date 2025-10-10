#ifndef __RADMOON3_H_
#define __RADMOON3_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoon3/radmoon3settings.h"

namespace icsneo {

class RADMoon3 : public Device {
public:
	// Serial numbers start with R3
	// USB PID is 0x110D, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADMoon3, DeviceType::RADMoon3, "R3");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::ETHERNET_01,
			Network::NetID::AE_01,
			Network::NetID::MDIO_01
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	bool isOnlineSupported() const override { return false; }

	bool supportsTC10() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADMoon3;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADMoon3_MCHIP, true, "MCHIP", "radmoon3_mchip_ief", 0, FirmwareType::IEF},
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADMoon3_MCHIP, BootloaderCommunication::RED)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}
protected:
	RADMoon3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoon3Settings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.align16bit = true;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportEthPhy = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }
	
};

}

#endif // __cplusplus

#endif