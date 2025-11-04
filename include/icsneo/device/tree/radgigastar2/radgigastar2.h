#ifndef __RADGIGASTAR2_H_
#define __RADGIGASTAR2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgigastar2/radgigastar2settings.h"

namespace icsneo
{

class RADGigastar2 : public Device {
public:
	enum class FirmwareVariant {
		T1Sx6_CANx1_LINx16 = 0,
		T1Sx8_CANx4_LINx6 = 1,
		Invalid = 2
	};

	// Serial numbers start with GT
	// USB PID is 0x1210, standard driver is DXX
	// Ethernet MAC allocation is 0x22, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGigastar2, DeviceType::RADGigastar2, "GT");

	static const std::vector<Network> &GetSupportedNetworks()
	{
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,

			Network::NetID::AE_01,
			Network::NetID::AE_02,
			Network::NetID::AE_03,
			Network::NetID::AE_04,
			Network::NetID::AE_05,
			Network::NetID::AE_06,
			Network::NetID::AE_07,
			Network::NetID::AE_08,
			Network::NetID::AE_09,
			Network::NetID::AE_10,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04,
			Network::NetID::LIN_05,
			Network::NetID::LIN_06,
			Network::NetID::LIN_07,
			Network::NetID::LIN_08,
			Network::NetID::LIN_09,
			Network::NetID::LIN_10,
			Network::NetID::LIN_11,
			Network::NetID::LIN_12,
			Network::NetID::LIN_13,
			Network::NetID::LIN_14,
			Network::NetID::LIN_15,
			Network::NetID::LIN_16,

			Network::NetID::I2C_01,
			Network::NetID::I2C_02,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,

			Network::NetID::SPI_01,
			Network::NetID::SPI_02,
			Network::NetID::SPI_03,
			Network::NetID::SPI_04,
			Network::NetID::SPI_05,
			Network::NetID::SPI_06,
			Network::NetID::SPI_07,
			Network::NetID::SPI_08,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }

	bool supportsTC10() const override { return true; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADGigastar2;
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

				if(component.identifier == static_cast<uint32_t>(ChipID::RADGigastar2_ZYNQ)) {
					if(component.componentInfo > 2) {
						return FirmwareVariant::Invalid;
					}
					auto res = static_cast<FirmwareVariant>(component.componentInfo);
					if(variantToFlash == FirmwareVariant::Invalid) {
						variantToFlash = res; // Set the variantToFlash if it hasn't been set yet, we always flash the same firmware variant as the current if it is unspecified
					}
					return res;
				}
			}
		}
		return FirmwareVariant::Invalid;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> linChips = {
			{ChipID::RADGigastar2_ZYNQ, true, "ZCHIP", "RADGigastar2_T1S_LIN_SW_bin", 1, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_ZYNQ, true, "USB ZCHIP", "RADGigastar_USBz_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7010_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7010_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7007S_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7007s_SW_bin", 2, FirmwareType::Zip},
		};

		static std::vector<ChipInfo> t1sChips = {
			{ChipID::RADGigastar2_ZYNQ, true, "ZCHIP", "RADGigastar2_T1S_CAN_SW_bin", 1, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_ZYNQ, true, "USB ZCHIP", "RADGigastar_USBz_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7010_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7010_SW_bin", 2, FirmwareType::Zip},
			{ChipID::RADGigastar_USBZ_Z7007S_ZYNQ, false, "USB ZCHIP", "RADGigastar_USBz_Z7007s_SW_bin", 2, FirmwareType::Zip},
		};
		
		if(variantToFlash == FirmwareVariant::T1Sx8_CANx4_LINx6) {
			return t1sChips;
		}
		return linChips; // Assume linChips even if variantToFlash is invalid
	}

	BootloaderPipeline getBootloader() override {
		auto chipVersions = getChipVersions();
		auto mainChip = std::find_if(chipVersions.begin(), chipVersions.end(), [](const auto& chip) { return chip.name == "ZCHIP"; });
		auto usbChip = std::find_if(chipVersions.begin(), chipVersions.end(), [](const auto& chip) { return chip.name == "USB ZCHIP"; });

		ChipID mainChipID;
		if(mainChip != chipVersions.end()) {
			mainChipID = mainChip->id;
		} else if(usbChip != chipVersions.end()) {
			mainChipID = usbChip->id;
		} else {
			return {};
		}

		BootloaderPipeline pipeline;
		for(const auto& version : chipVersions) {
			pipeline.add<FlashPhase>(version.id, BootloaderCommunication::RADMultiChip);
		}
		pipeline.add<EnterApplicationPhase>(mainChipID);
		pipeline.add<WaitPhase>(std::chrono::milliseconds(3000));
		pipeline.add<ReconnectPhase>();
		return pipeline;
	}

	std::vector<VersionReport> getChipVersions(bool refreshComponents = true) override {
		if(variantToFlash == FirmwareVariant::Invalid) {
			getCurrentVariant();
		}
		
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
				chipVersions.push_back({ChipID::RADGigastar2_ZYNQ, "ZCHIP", appVersions[0]->major, appVersions[0]->minor, 0, 0});
			}
		} else {
			if(MainPeriphIndex < appVersions.size()) {
				if(appVersions[MainPeriphIndex]) {
					chipVersions.push_back({ChipID::RADGigastar2_ZYNQ, "ZCHIP", appVersions[MainPeriphIndex]->major, appVersions[MainPeriphIndex]->minor, 0, 0});
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
	RADGigastar2(neodevice_t neodevice, const driver_factory_t &makeDriver) : Device(neodevice)
	{
		initialize<RADGigastar2Settings>(makeDriver);
	}

	void setupPacketizer(Packetizer &packetizer) override
	{
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupDecoder(Decoder &decoder) override
	{
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupEncoder(Encoder &encoder) override
	{
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
		encoder.supportEthPhy = true;
	}

	void setupSupportedRXNetworks(std::vector<Network> &rxNetworks) override
	{
		for (auto &netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network> &txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void handleDeviceStatus(const std::shared_ptr<RawMessage> &message) override
	{
		if (message->data.size() < sizeof(radgigastar2_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgigastar2_status_t *status = reinterpret_cast<const radgigastar2_status_t *>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override
	{
		return 512 * 4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override
	{
		return 0;
	}
	
	size_t getDiskCount() const override 
	{
		return 1;
	}
};

}

#endif // __cplusplus

#endif