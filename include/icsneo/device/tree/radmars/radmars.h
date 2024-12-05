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
	// USB PID is 0x1203, standard driver is FTDI3
	// Ethernet MAC allocation is 0x0A, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADMars, _icsneo_devicetype_t::icsneo_devicetype_rad_mars, "GL");

	size_t getEthernetActivationLineCount() const override { return 1; }

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
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_mscan,
			Network::_icsneo_netid_t::HSCAN2,
			Network::_icsneo_netid_t::HSCAN3,
			Network::_icsneo_netid_t::HSCAN4,
			Network::_icsneo_netid_t::HSCAN5,
			Network::_icsneo_netid_t::HSCAN6,
			Network::_icsneo_netid_t::HSCAN7,

			Network::_icsneo_netid_t::Ethernet,
			Network::_icsneo_netid_t::Ethernet2,
			Network::_icsneo_netid_t::Ethernet3,

			Network::_icsneo_netid_t::icsneo_netid_lin,

			Network::_icsneo_netid_t::FlexRay1a,
			Network::_icsneo_netid_t::FlexRay1b,

			Network::_icsneo_netid_t::I2C,

			Network::_icsneo_netid_t::MDIO1,
			Network::_icsneo_netid_t::MDIO2,
		};
		rxNetworks.insert(rxNetworks.end(), supportedRxNetworks.begin(), supportedRxNetworks.end());
	}

	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override {
		static std::vector<Network> supportedTxNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_mscan,
			Network::_icsneo_netid_t::HSCAN2,
			Network::_icsneo_netid_t::HSCAN3,
			Network::_icsneo_netid_t::HSCAN4,
			Network::_icsneo_netid_t::HSCAN5,
			Network::_icsneo_netid_t::HSCAN6,
			Network::_icsneo_netid_t::HSCAN7,

			Network::_icsneo_netid_t::Ethernet,
			Network::_icsneo_netid_t::Ethernet2,
			Network::_icsneo_netid_t::Ethernet3,

			Network::_icsneo_netid_t::icsneo_netid_lin,

			// FlexRay is Receive Only
			
			Network::_icsneo_netid_t::I2C,

			Network::_icsneo_netid_t::MDIO1,
			Network::_icsneo_netid_t::MDIO2,
		};
		txNetworks.insert(txNetworks.end(), supportedTxNetworks.begin(), supportedTxNetworks.end());
	}

	void handleDeviceStatus(const std::shared_ptr<InternalMessage>& message) override {
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