#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/neovifire2/neovifire2settings.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	// Serial numbers start with CY
	// USB PID is 0x1000, standard driver is FTDI
	// Ethernet MAC allocation is 0x04, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(NeoVIFIRE2, DeviceType::FIRE2, "CY");

	enum class SKU {
		Standard,
		AP1200A, // Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::MSCAN,
			Network::NetID::HSCAN2,
			Network::NetID::HSCAN3,
			Network::NetID::HSCAN4,
			Network::NetID::HSCAN5,
			Network::NetID::HSCAN6,
			Network::NetID::HSCAN7,

			Network::NetID::LSFTCAN,
			Network::NetID::LSFTCAN2,

			Network::NetID::SWCAN,
			Network::NetID::SWCAN2,

			Network::NetID::Ethernet,

			Network::NetID::LIN,
			Network::NetID::LIN2,
			Network::NetID::LIN3,
			Network::NetID::LIN4,

			Network::NetID::ISO9141,
			Network::NetID::ISO9141_2,
			Network::NetID::ISO9141_3,
			Network::NetID::ISO9141_4
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

protected:
	NeoVIFIRE2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRE2Settings>(makeDriver);
	}

	void setupSettings(IDeviceSettings& ssettings) override {
		if(com->driver->isEthernet()) {
			// TODO Check firmware version, old firmwares will reset Ethernet settings on settings send
			ssettings.readonly = true;
		}
	}

	bool currentDriverSupportsDFU() const override { return com->driver->isEthernet(); }

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
};

}

#endif // __cplusplus

#endif