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
	// USB PID is 0x1204, standard driver is FTDI3
	// Ethernet MAC allocation is 0x0F, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGigastar, _icsneo_devicetype_t::icsneo_devicetype_rad_gigastar, "GS");

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool getEthPhyRegControlSupported() const override { return true; }
	bool supportsTC10() const override { return true; }
	
protected:
	RADGigastar(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGigastarSettings>(makeDriver);
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
			_icsneo_netid_t::icsneo_netid_hscan,
			_icsneo_netid_t::icsneo_netid_mscan,
			_icsneo_netid_t::icsneo_netid_hscan2,
			_icsneo_netid_t::icsneo_netid_hscan3,
			_icsneo_netid_t::icsneo_netid_hscan4,
			_icsneo_netid_t::icsneo_netid_hscan5,

			_icsneo_netid_t::icsneo_netid_ethernet,
			_icsneo_netid_t::icsneo_netid_ethernet2,

			_icsneo_netid_t::icsneo_netid_op_ethernet1,
			_icsneo_netid_t::icsneo_netid_op_ethernet2,

			_icsneo_netid_t::icsneo_netid_lin,

			_icsneo_netid_t::icsneo_netid_flexray1a,
			_icsneo_netid_t::icsneo_netid_flexray1b,

			_icsneo_netid_t::icsneo_netid_i2c,
			_icsneo_netid_t::icsneo_netid_i2c2,
			_icsneo_netid_t::icsneo_netid_i2c3,

			_icsneo_netid_t::icsneo_netid_mdio1,
			_icsneo_netid_t::icsneo_netid_mdio2,
		};
		rxNetworks.insert(rxNetworks.end(), supportedRxNetworks.begin(), supportedRxNetworks.end());
	}

	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override {
		static std::vector<Network> supportedTxNetworks = {
			_icsneo_netid_t::icsneo_netid_hscan,
			_icsneo_netid_t::icsneo_netid_mscan,
			_icsneo_netid_t::icsneo_netid_hscan2,
			_icsneo_netid_t::icsneo_netid_hscan3,
			_icsneo_netid_t::icsneo_netid_hscan4,
			_icsneo_netid_t::icsneo_netid_hscan5,

			_icsneo_netid_t::icsneo_netid_ethernet,
			_icsneo_netid_t::icsneo_netid_ethernet2,

			_icsneo_netid_t::icsneo_netid_op_ethernet1,
			_icsneo_netid_t::icsneo_netid_op_ethernet2,

			_icsneo_netid_t::icsneo_netid_lin,

			// FlexRay is Receive Only

			_icsneo_netid_t::icsneo_netid_i2c,
			_icsneo_netid_t::icsneo_netid_i2c2,
			_icsneo_netid_t::icsneo_netid_i2c3,

			_icsneo_netid_t::icsneo_netid_mdio1,
			_icsneo_netid_t::icsneo_netid_mdio2,
		};
		txNetworks.insert(txNetworks.end(), supportedTxNetworks.begin(), supportedTxNetworks.end());
	}

	void handleDeviceStatus(const std::shared_ptr<InternalMessage>& message) override {
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
};

}

#endif // __cplusplus

#endif