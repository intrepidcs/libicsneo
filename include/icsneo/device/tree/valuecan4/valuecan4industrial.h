#ifndef __VALUECAN4INDUSTRIAL_H_
#define __VALUECAN4INDUSTRIAL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4industrialsettings.h"

namespace icsneo {

class ValueCAN4Industrial : public ValueCAN4 {
public:
	// Serial numbers start with IV for Industrial
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	// Ethernet MAC allocation is 0x12, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(ValueCAN4Industrial, DeviceType::VCAN4_IND, "IV");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01
		};
		return supportedNetworks;
	}

	ProductID getProductID() const override {
		return ProductID::ValueCAN4Industrial;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::ValueCAN4Industrial_MCHIP, true, "MCHIP", "vcan4_ind_mchip_ief", 0, FirmwareType::IEF}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::ValueCAN4Industrial_MCHIP, BootloaderCommunication::RED)
			.add<EnterApplicationPhase>(ChipID::ValueCAN4Industrial_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}
protected:
	ValueCAN4Industrial(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4IndustrialSettings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void setupPacketizer(Packetizer& packetizer) override {
		ValueCAN4::setupPacketizer(packetizer);
		packetizer.align16bit = !com->driver->isEthernet();
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}

#endif // __cplusplus

#endif