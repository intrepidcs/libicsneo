#ifndef __RADGALAXY2_H_
#define __RADGALAXY2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgalaxy2/radgalaxy2settings.h"

namespace icsneo {

class RADGalaxy2 : public Device {
public:
	// Serial numbers start with G2
	// Ethernet MAC allocation is 0x17, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGalaxy2, icsneo_devicetype_rad_galaxy2, "G2");

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			icsneo_netid_hscan,
			icsneo_netid_mscan,
			icsneo_netid_hscan2,
			icsneo_netid_hscan3,
			icsneo_netid_hscan4,
			icsneo_netid_hscan5,
			icsneo_netid_hscan6,
			icsneo_netid_hscan7,

			icsneo_netid_lin,
			icsneo_netid_lin2,

			icsneo_netid_ethernet,
			icsneo_netid_ethernet2,
			icsneo_netid_ethernet3,

			icsneo_netid_op_ethernet1,
			icsneo_netid_op_ethernet2,
			icsneo_netid_op_ethernet3,
			icsneo_netid_op_ethernet4,
			icsneo_netid_op_ethernet5,
			icsneo_netid_op_ethernet6,
			icsneo_netid_op_ethernet7,
			icsneo_netid_op_ethernet8,
			icsneo_netid_op_ethernet9,
			icsneo_netid_op_ethernet10,
			icsneo_netid_op_ethernet11,
			icsneo_netid_op_ethernet12,

			icsneo_netid_iso9141,
			icsneo_netid_iso9141_2,

			icsneo_netid_mdio1,
			icsneo_netid_mdio2,
			icsneo_netid_mdio3,
			icsneo_netid_mdio4,
			icsneo_netid_mdio5,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

	bool supportsTC10() const override { return true; }

protected:
	RADGalaxy2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGalaxy2Settings, Disk::ExtExtractorDiskReadDriver, Disk::NeoMemoryDiskDriver>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
		encoder.supportEthPhy = true;
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
		if(message->data.size() < sizeof(radgalaxy2_status_t))
			return;
		std::lock_guard<std::mutex> lk(ioMutex);
		const radgalaxy2_status_t* status = reinterpret_cast<const radgalaxy2_status_t*>(message->data.data());
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