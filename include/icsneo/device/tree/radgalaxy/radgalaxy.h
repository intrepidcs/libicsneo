#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/device/tree/radgalaxy/radgalaxysettings.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	// Ethernet MAC allocation is 0x03, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE(RADGalaxy, DeviceType::RADGalaxy, "RG");

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

			Network::NetID::SWCAN,
			Network::NetID::SWCAN2,

			Network::NetID::LIN,

			Network::NetID::Ethernet,
			Network::NetID::Ethernet2,
			
			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2,
			Network::NetID::OP_Ethernet3,
			Network::NetID::OP_Ethernet4,
			Network::NetID::OP_Ethernet5,
			Network::NetID::OP_Ethernet6,
			Network::NetID::OP_Ethernet7,
			Network::NetID::OP_Ethernet8,
			Network::NetID::OP_Ethernet9,
			Network::NetID::OP_Ethernet10,
			Network::NetID::OP_Ethernet11,
			Network::NetID::OP_Ethernet12,

			Network::NetID::MDIO1,
			Network::NetID::MDIO2,
			Network::NetID::MDIO3,
			Network::NetID::MDIO4,
			Network::NetID::MDIO5,
		};
		return supportedNetworks;
	}

	size_t getEthernetActivationLineCount() const override { return 1; }

protected:
	RADGalaxy(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADGalaxySettings>(makeDriver);
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

	void handleDeviceStatus(const std::shared_ptr<RawMessage>& message) override {
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