#ifndef __RADGIGASTAR_H_
#define __RADGIGASTAR_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgigastar/radgigastarsettings.h"

namespace icsneo {

class RADGigastar : public Device {
public:
	// Serial numbers start with GS
	// USB PID is 0x1204, standard driver is DXX
	// Ethernet MAC allocation is 0x0F, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGigastar, DeviceType::RADGigastar, "GS");

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }
	bool supportsTC10() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADGigastar;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::RADGigastar_ZYNQ, true, "ZCHIP", "RADGigastar_SW_bin", 1, FirmwareType::Zip},
			{ChipID::RADGigastar_FFG_ZYNQ, false, "ZCHIP", "RADGigastar_FFG_SW_bin", 1, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_ZYNQ, true, "USB ZCHIP", "RADGigastar_USBz_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7010_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7010_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7007S_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7007s_SW_bin", 2, FirmwareType::Zip},
		};
		return chips;
	}

	BootloaderPipeline getBootloader() override {
		if(supportsComponentVersions()) {
			// main: 16.1, usb: 14.0
			auto chipVersions = getChipVersions();
			auto mainVersion = std::find_if(chipVersions.begin(), chipVersions.end(), [](const auto& ver) { return ver.name == "ZCHIP"; });
			auto usbVersion = std::find_if(chipVersions.begin(), chipVersions.end(), [](const auto& ver) { return ver.name == "USB ZCHIP"; });
			
			bool useNewBootloader = false;
			if(mainVersion != chipVersions.end()) {
				static constexpr uint8_t NewBootloaderMajor = 16;
				static constexpr uint8_t NewBootloaderMinor = 1;
				if(
					mainVersion->major > NewBootloaderMajor || 
					(mainVersion->major == NewBootloaderMajor && mainVersion->minor >= NewBootloaderMinor)
				) {
					useNewBootloader = true;
				}
			} else if(usbVersion != chipVersions.end()) {
				static constexpr uint8_t NewBootloaderMajorUSB = 14;
				static constexpr uint8_t NewBootloaderMinorUSB= 0;
				if(
					usbVersion->major > NewBootloaderMajorUSB || 
					(usbVersion->major == NewBootloaderMajorUSB && usbVersion->minor >= NewBootloaderMinorUSB)
				) {
					useNewBootloader = true;
				}				
			}
			if(useNewBootloader) {
				BootloaderPipeline pipeline;
				for(const auto& version : chipVersions) {
					pipeline.add<FlashPhase>(version.id, BootloaderCommunication::RADMultiChip);
				}
				pipeline.add<ReconnectPhase>();
				pipeline.add<WaitPhase>(std::chrono::milliseconds(3000));
				return pipeline;
			}
		}
		// If we've reached this point, then we use the legacy flashing
		if(com->driver->isEthernet()) {
			return BootloaderPipeline()
				.add<FlashPhase>(ChipID::RADGigastar_ZYNQ, BootloaderCommunication::RAD)
				.add<ReconnectPhase>()
				.add<WaitPhase>(std::chrono::milliseconds(3000));
		}
		return BootloaderPipeline()
			.add<FlashPhase>(ChipID::RADGigastar_USBZ_ZYNQ, BootloaderCommunication::RAD)
			.add<ReconnectPhase>()
			.add<WaitPhase>(std::chrono::milliseconds(3000));
	}

	std::vector<VersionReport> getChipVersions(bool refreshComponents = true) override {
		if(refreshComponents) {
			refreshComponentVersions();
		}
		if(supportsComponentVersions()) {
			return Device::getChipVersions(false);
		}
		static constexpr size_t MainPeriphIndex = 1;
		static constexpr size_t USBPeriphIndex = 2;

		std::vector<VersionReport> chipVersions;
		auto& appVersions = getVersions();

		if(appVersions.size() < 3 && !appVersions.empty()) {
			if(appVersions[0]) {
				chipVersions.push_back({ChipID::RADGigastar_ZYNQ, "ZCHIP", appVersions[0]->major, appVersions[0]->minor, 0, 0});
			}
		} else {
			if(MainPeriphIndex < appVersions.size()) {
				if(appVersions[MainPeriphIndex]) {
					chipVersions.push_back({ChipID::RADGigastar_ZYNQ, "ZCHIP", appVersions[MainPeriphIndex]->major, appVersions[MainPeriphIndex]->minor, 0, 0});
				}
			}
			if(USBPeriphIndex < appVersions.size()) {
				if(appVersions[USBPeriphIndex]) {
					chipVersions.push_back({ChipID::RADGigastar_USBZ_ZYNQ, "USB ZCHIP", appVersions[USBPeriphIndex]->major, appVersions[USBPeriphIndex]->minor, 0, 0});
				}
			}
		}
		return chipVersions;
	}

protected:
	RADGigastar(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGigastarSettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
		encoder.supportEthPhy = true;
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		static std::vector<Network> supportedRxNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,

			Network::NetID::AE_01,
			Network::NetID::AE_02,

			Network::NetID::LIN_01,

			Network::NetID::FLEXRAY_01A,
			Network::NetID::FLEXRAY_01B,

			Network::NetID::I2C_01,
			Network::NetID::I2C_02,
			Network::NetID::I2C_03,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
		};
		rxNetworks.insert(rxNetworks.end(), supportedRxNetworks.begin(), supportedRxNetworks.end());
	}

	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override {
		static std::vector<Network> supportedTxNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,

			Network::NetID::AE_01,
			Network::NetID::AE_02,

			Network::NetID::LIN_01,

			// FlexRay is Receive Only

			Network::NetID::I2C_01,
			Network::NetID::I2C_02,
			Network::NetID::I2C_03,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
		};
		txNetworks.insert(txNetworks.end(), supportedTxNetworks.begin(), supportedTxNetworks.end());
	}

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
		if(message->data.size() < sizeof(radgigastar_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgigastar_status_t* status = reinterpret_cast<const radgigastar_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}

	size_t getDiskCount() const override {
		return 1;
	}
};

}

#endif // __cplusplus

#endif