#ifndef __RADWBMS_H_
#define __RADWBMS_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radwbms/radwbmssettings.h"

namespace icsneo {

class RADwBMS : public Device {
public:
	enum class FirmwareVariant {
		Invalid = 0,
		WIL_1_1 = 0x0101000C,
		WIL_2_0 = 0x02000058,
		WIL_2_0_9_26 = 0x0200091A,
		WIL_2_1 = 0x02010047,
		WIL_2_2 = 0x0202000F,
		WIL_2_3 = 0x0203020B,
		WIL_3_1_0_9 = 0x03010009,
		WIL_3_2_0 = 0x03020000,
		WIL_3_3_0_27 = 0x0303001B,
	};

	// Serial numbers start with BS
	// USB PID is 0x110B, standard driver is CDACM
	// Ethernet MAC allocation is 0x1B, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADwBMS, DeviceType::RADwBMS, "BS");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,
			Network::NetID::ETHERNET_01,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool supportsReboot() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADwBMS;
	}

	FirmwareVariant variantToFlash = FirmwareVariant::Invalid;
	void setVariantToFlash(FirmwareVariant variant) {
		variantToFlash = variant;
	}

	FirmwareVariant getCurrentVariant() {
		if(supportsComponentVersions()) {
			refreshComponentVersions();

			const auto& components = getComponentVersions();
			for(const auto& component : components) {
				if(!component.valid) {
					continue;
				}

				if(component.identifier == static_cast<uint32_t>(ChipID::RADBMS_WIL)) {
					FirmwareVariant res = FirmwareVariant::Invalid;
					switch(static_cast<FirmwareVariant>(component.dotVersion)) {
						case FirmwareVariant::WIL_1_1:
						case FirmwareVariant::WIL_2_0:
						case FirmwareVariant::WIL_2_0_9_26:
						case FirmwareVariant::WIL_2_1:
						case FirmwareVariant::WIL_2_2:
						case FirmwareVariant::WIL_2_3:
						case FirmwareVariant::WIL_3_1_0_9:
						case FirmwareVariant::WIL_3_2_0:
						case FirmwareVariant::WIL_3_3_0_27:
							res = static_cast<FirmwareVariant>(component.dotVersion);
							break;
						default:
							res = FirmwareVariant::Invalid;
							break;
					}
					if(variantToFlash == FirmwareVariant::Invalid) {
						// Set the variantToFlash if it hasn't been set yet, we always flash the same firmware variant as the current if it is unspecified
						variantToFlash = res;
					}
					return res;
				}
			}
		}
		return FirmwareVariant::Invalid;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		switch (variantToFlash) {
			case FirmwareVariant::WIL_1_1: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_1_1_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_2_0: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_2_0_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_2_0_9_26: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_2_0_9_26_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_2_1: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_2_1_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_2_2: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_2_2_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_2_3: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_2_3_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_3_1_0_9: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_3_1_0_9_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_3_2_0: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_3_2_0_ief", 0, FirmwareType::IEF}};
				return chips;
			}
			case FirmwareVariant::WIL_3_3_0_27: {
				static std::vector<ChipInfo> chips = {{ChipID::RADBMS_MCHIP, true, "MCHIP", "rad_bms_mchip_WIL_3_3_0_27_ief", 0, FirmwareType::IEF}};
				return chips;
			}
		}
		// Return empty chip information if the WIL version is not set
		static std::vector<ChipInfo> chips =  {};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::RADBMS_MCHIP, BootloaderCommunication::RED)
			.add<EnterApplicationPhase>(ChipID::RADBMS_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}

	std::vector<VersionReport> getChipVersions(bool refreshComponents = true) override {
		if(variantToFlash == FirmwareVariant::Invalid) {
			getCurrentVariant();
		}
		return Device::getChipVersions(refreshComponents);
	}
	
protected:
	RADwBMS(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADwBMSSettings, Disk::NeoMemoryDiskDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 2048 * 512;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	bool supportsEraseMemory() const override {
		return true;
	}
};

}; // namespace icsneo

#endif // __RADWBMS_H_
