#ifndef __RADMARS_H_
#define __RADMARS_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radmars/radmarssettings.h"

namespace icsneo {

class RADMars : public Device {
public:
	// Serial numbers start with GL (previously, RAD-Gigalog)
	// USB PID is 0x1203, standard driver is DXX
	// Ethernet MAC allocation is 0x0A, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADMars, DeviceType::RADMars, "GL");

	size_t getEthernetActivationLineCount() const override { return 1; }
	bool supportsGPTP() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADMars;
	}
protected:
	RADMars(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMarsSettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
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
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		static std::vector<Network> supportedRxNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,
			Network::NetID::DWCAN_04,
			Network::NetID::DWCAN_05,
			Network::NetID::DWCAN_06,
			Network::NetID::DWCAN_07,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,
			Network::NetID::ETHERNET_03,

			Network::NetID::LIN_01,

			Network::NetID::FLEXRAY_01A,
			Network::NetID::FLEXRAY_01B,

			Network::NetID::I2C_01,

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
			Network::NetID::DWCAN_06,
			Network::NetID::DWCAN_07,

			Network::NetID::ETHERNET_01,
			Network::NetID::ETHERNET_02,
			Network::NetID::ETHERNET_03,

			Network::NetID::LIN_01,

			// FlexRay is Receive Only
			
			Network::NetID::I2C_01,

			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
		};
		txNetworks.insert(txNetworks.end(), supportedTxNetworks.begin(), supportedTxNetworks.end());
	}

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
		if(message->data.size() < sizeof(radmars_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radmars_status_t* status = reinterpret_cast<const radmars_status_t*>(message->data.data());
		ethActivationStatus = status->ethernetActivationLineEnabled;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*4;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif // __cplusplus

#endif