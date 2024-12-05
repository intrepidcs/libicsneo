#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgalaxy/radgalaxysettings.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	// Ethernet MAC allocation is 0x03, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGalaxy, _icsneo_devicetype_t::icsneo_devicetype_rad_galaxy, "RG");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::_icsneo_netid_t::icsneo_netid_hscan,
			Network::_icsneo_netid_t::icsneo_netid_mscan,
			Network::_icsneo_netid_t::icsneo_netid_hscan2,
			Network::_icsneo_netid_t::icsneo_netid_hscan3,
			Network::_icsneo_netid_t::icsneo_netid_hscan4,
			Network::_icsneo_netid_t::icsneo_netid_hscan5,
			Network::_icsneo_netid_t::icsneo_netid_hscan6,
			Network::_icsneo_netid_t::icsneo_netid_hscan7,

			Network::_icsneo_netid_t::icsneo_netid_swcan,
			Network::_icsneo_netid_t::icsneo_netid_swcan2,

			Network::_icsneo_netid_t::icsneo_netid_lin,

			Network::_icsneo_netid_t::icsneo_netid_ethernet,
			Network::_icsneo_netid_t::icsneo_netid_ethernet2,
			
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet1,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet2,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet3,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet4,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet5,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet6,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet7,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet8,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet9,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet10,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet11,
			Network::_icsneo_netid_t::icsneo_netid_op_ethernet12,

			Network::_icsneo_netid_t::icsneo_netid_mdio1,
			Network::_icsneo_netid_t::icsneo_netid_mdio2,
			Network::_icsneo_netid_t::icsneo_netid_mdio3,
			Network::_icsneo_netid_t::icsneo_netid_mdio4,
			Network::_icsneo_netid_t::icsneo_netid_mdio5,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

protected:
	RADGalaxy(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGalaxySettings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	void handleDeviceStatus(const std::shared_ptr<InternalMessage>& message) override {
		if(message->data.size() < sizeof(radgalaxy_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgalaxy_status_t* status = reinterpret_cast<const radgalaxy_status_t*>(message->data.data());
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