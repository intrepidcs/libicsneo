#ifndef __RADARGOS_H_
#define __RADARGOS_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radargos/radargossettings.h"

namespace icsneo {

class RADArgos : public Device {
public:
	// Serial numbers start with GE
	// USB PID is 0x110E, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADArgos, DeviceType::RADArgos, "GE");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::MDIO_01,
		};
		return supportedNetworks;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	bool isOnlineSupported() const override { return false; }

	bool supportsTC10() const override { return true; }

	ProductID getProductID() const override { return ProductID::RADArgos; }

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADArgos_MCHIP, true, "MCHIP", "radargos_mchip_ief", 0, FirmwareType::IEF},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADArgos_MCHIP, BootloaderCommunication::RED)
			.add<EnterApplicationPhase>(ChipID::RADArgos_MCHIP)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}

protected:
	RADArgos(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADArgosSettings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.align16bit = true;
	}

	void setupEncoder(Encoder& encoder) override {
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

#endif // __RADARGOS_H_
