#ifndef __VALUECAN4_2EL_H_
#define __VALUECAN4_2EL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4.h"
#include "icsneo/device/tree/valuecan4/settings/valuecan4-2elsettings.h"

namespace icsneo {

class ValueCAN4_2EL : public ValueCAN4 {
public:
	// Serial numbers start with VE for 4-2EL
	// USB PID is 0x1101 (shared by all ValueCAN 4s), standard driver is CDCACM
	// Ethernet MAC allocation is 0x0B, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(ValueCAN4_2EL, DeviceType::VCAN4_2EL, "VE");

	enum class SKU {
		Standard,
		AP04E0A_D26, // HDB26, USB A, and Keysight Branding
		AP04E0A_MUL, // Multi-connectors, USB A, and Keysight Branding
		AP04E0A_OBD, // OBD, USB A, and Keysight Branding
	};

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::AP04E0A_D26;
			case 'B':
				return SKU::AP04E0A_MUL;
			case 'C':
				return SKU::AP04E0A_OBD;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::AP04E0A_D26:
				return "Keysight AP04E0A-D26";
			case SKU::AP04E0A_MUL:
				return "Keysight AP04E0A-MUL";
			case SKU::AP04E0A_OBD:
				return "Keysight AP04E0A-OBD";
		}
		return Device::getProductName();
	}

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
		return ProductID::ValueCAN4_2EL_4;
	}

	const std::vector<ChipInfo>& getChipInfo() const override {
		static std::vector<ChipInfo> chips = {
			{ChipID::ValueCAN4_2EL_MCHIP, true, "MCHIP", "vcan44_mchip_ief", 0, FirmwareType::IEF},
			{ChipID::ValueCAN4_4_MCHIP, true, "MCHIP", "vcan44_mchip_ief", 0, FirmwareType::IEF}
		};
		return chips;
	}
	
	BootloaderPipeline getBootloader() override {
		return BootloaderPipeline()
			.add<EnterBootloaderPhase>()
			.add<FlashPhase>(ChipID::ValueCAN4_4_MCHIP, BootloaderCommunication::RED)
			.add<EnterApplicationPhase>(ChipID::ValueCAN4_4_MCHIP)
			.add<WaitPhase>(std::chrono::milliseconds(3000))
			.add<ReconnectPhase>();
	}


protected:
	ValueCAN4_2EL(neodevice_t neodevice, const driver_factory_t& makeDriver) : ValueCAN4(neodevice) {
		initialize<ValueCAN4_2ELSettings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	size_t getEthernetActivationLineCount() const override { return 1; }

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
		if(message->data.size() < sizeof(valuecan4_2el_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const valuecan4_2el_status_t* status = reinterpret_cast<const valuecan4_2el_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	bool currentDriverSupportsDFU() const override { return !com->driver->isEthernet(); }

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