#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/tree/neovifire2/neovifire2settings.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::FIRE2;
	static constexpr const char* SERIAL_START = "CY";

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

	size_t getEthernetActivationLineCount() const override { return 1; }
	size_t getUSBHostPowerCount() const override { return 1; }
	bool getBackupPowerSupported() const override { return true; }

protected:
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void handleDeviceStatus(const std::shared_ptr<Message>& message) override {
		if(!message || message->data.size() < sizeof(neovifire2_status_t))
			return;
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