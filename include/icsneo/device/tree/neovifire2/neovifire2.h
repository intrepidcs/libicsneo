#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/neovifire2/neovifire2settings.h"
#include "icsneo/disk/neomemorydiskdriver.h"
namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	enum class CoreChipVariant {
		Core = 0,
		Core_SG4 = 1,
		Invalid = 2
	};

	// Serial numbers start with CY
	// USB PID is 0x1000, standard driver is DXX
	// Ethernet MAC allocation is 0x04, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE2, DeviceType::FIRE2, "CY");

	enum class SKU {
		Standard,
		AP1200A, // Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,
			Network::NetID::DWCAN_06,
			Network::NetID::DWCAN_07,

			Network::NetID::LSFTCAN_01,
			Network::NetID::LSFTCAN_02,

			Network::NetID::SWCAN_01,
			Network::NetID::SWCAN_02,

			Network::NetID::ETHERNET_01,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04,

			Network::NetID::ISO9141_01,
			Network::NetID::ISO9141_02,
			Network::NetID::ISO9141_03,
			Network::NetID::ISO9141_04
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::AP1200A;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP1200A:
				return "Keysight AP1200A";
		}
		return Device::getProductName();
	}

	size_t getEthernetActivationLineCount() const override { return 1; }
	size_t getUSBHostPowerCount() const override { return 1; }
	bool getBackupPowerSupported() const override { return true; }
	std::vector<MiscIO> getMiscIO() const override {
		return {
			{5, true, true, false},
			{6, true, true, false}
		};
	}
	std::vector<MiscIO> getEMiscIO() const override {
		return {
			{1, true, true, true},
			{2, true, true, true}
		};
	}

	ProductID getProductID() const override {
		return ProductID::neoVIFIRE2;
	}

	CoreChipVariant getCoreChipVariant() {
		if(!bootloaderVersion.has_value()) {
			const auto& hardwareInfo = getHardwareInfo(std::chrono::milliseconds(1000));
			if(!hardwareInfo) {
				chipVariant = CoreChipVariant::Invalid;
				return chipVariant;
			}
			setBootloaderVersion(hardwareInfo->bootloaderVersion);
		}
		if(bootloaderVersion->major >= CORE_SG4_BL_MAJOR_VERSION_CUTOFF) {
			chipVariant = CoreChipVariant::Core_SG4;
		} else {
			chipVariant = CoreChipVariant::Core;
		}
		return chipVariant;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::neoVIFIRE2_MCHIP, true, "MCHIP", "fire2_mchip_ief", 0, FirmwareType::IEF},
			{ChipID::neoVIFIRE2_ZYNQ, true, "ZCHIP", "fire2_zchip_ief", 1, FirmwareType::IEF},
			{ChipID::neoVIFIRE2_Core, true, "Core", "fire2_core", 2, FirmwareType::IEF},
		};

		static std::vector<ChipInfo> chipsSG4 = {
			{ChipID::neoVIFIRE2_MCHIP, true, "MCHIP", "fire2_mchip_ief", 0, FirmwareType::IEF},
			{ChipID::neoVIFIRE2_ZYNQ, true, "ZCHIP", "fire2_zchip_ief", 1, FirmwareType::IEF},
			{ChipID::neoVIFIRE2_Core_SG4, true, "Core", "fire2_core_sg4", 2, FirmwareType::IEF}
		};
		
		if(chipVariant == CoreChipVariant::Core_SG4) {
			return chipsSG4;
		}

		return chips; // Return the base chips even if the mode is invalid
	}

	BootloaderPipeline getBootloader() override {
		BootloaderPipeline pipeline;
		pipeline.add<EnterBootloaderPhase>()
				.add<FlashPhase>(ChipID::neoVIFIRE2_MCHIP, BootloaderCommunication::RED);
		if(chipVariant == CoreChipVariant::Core_SG4) {
			pipeline.add<FlashPhase>(ChipID::neoVIFIRE2_Core_SG4, BootloaderCommunication::REDCore, false, false);
		} else {
			pipeline.add<FlashPhase>(ChipID::neoVIFIRE2_Core, BootloaderCommunication::REDCore, false, false);
		}
		pipeline.add<FlashPhase>(ChipID::neoVIFIRE2_ZYNQ, BootloaderCommunication::RED, false, false)
				.add<EnterApplicationPhase>(ChipID::neoVIFIRE2_MCHIP)
				.add<ReconnectPhase>();
		return pipeline;
	}

	bool supportsSwVersionValidate() const override {
		return bootloaderVersion.has_value() && (bootloaderVersion->major > 4 || (bootloaderVersion->major == 4 && bootloaderVersion->minor >= 3));
	}

	std::vector<VersionReport> getChipVersions(bool refreshComponents = true) override {
		if(chipVariant == CoreChipVariant::Invalid) {
			getCoreChipVariant();
		}
		
		if(refreshComponents) {
			refreshComponentVersions();
		}
		return Device::getChipVersions();
	}
	
protected:
	NeoVIFIRE2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRE2Settings, Disk::NeoMemoryDiskDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupSettings(IDeviceSettings& ssettings) override {
		if(com->driver->isEthernet()) {
			// TODO Check firmware version, old firmwares will reset Ethernet settings on settings send
			ssettings.readonly = true;
		}
	}

	bool currentDriverSupportsDFU() const override { return !com->driver->isEthernet(); }

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.align16bit = !com->driver->isEthernet();
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

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
		if(message->data.size() < sizeof(neovifire2_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const neovifire2_status_t* status = reinterpret_cast<const neovifire2_status_t*>(message->data.data());
		backupPowerEnabled = status->backupPowerEnabled;
		backupPowerGood = status->backupPowerGood;
		ethActivationStatus = status->ethernetActivationLineEnabled;
		usbHostPowerStatus = status->usbHostPowerEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 4608 * 512;
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

private:
	CoreChipVariant chipVariant = CoreChipVariant::Invalid;
	constexpr static uint8_t CORE_SG4_BL_MAJOR_VERSION_CUTOFF = 5;
};

}

#endif // __cplusplus

#endif