#ifndef __RADEPSILONXL_H_
#define __RADEPSILONXL_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radepsilon/radepsilonsettings.h" // Shared settings with RADEpsilon

namespace icsneo {

class RADEpsilonXL : public Device {
public:
	// Serial numbers start with PX
	// USB PID is 0x1109, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADEpsilonXL, DeviceType::RADEpsilonXL, "PX");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01
		};
		return supportedNetworks;
	}
	
	bool supportsComponentVersions() const override { return true; }

	bool getEthPhyRegControlSupported() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADEpsilon;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADProxima_MCHIP, true, "MCHIP", "epsilon_mchip_ief", 0, FirmwareType::IEF},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADProxima_MCHIP, BootloaderCommunication::RED)
			.add<ReconnectPhase>();
	}

protected:
	RADEpsilonXL(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADEpsilonSettings, Disk::NeoMemoryDiskDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 14*1024*1024;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsEraseMemory() const override {
		return true;
	}

	size_t getDiskCount() const override {
		return 1;
	}
};

}; // namespace icsneo

#endif // __RADEPSILONXL_H_
