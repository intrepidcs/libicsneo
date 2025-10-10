#ifndef __RADSUPERMOON_H_
#define __RADSUPERMOON_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radsupermoon/radsupermoonsettings.h"

namespace icsneo {

class RADSupermoon : public Device {
public:
	// Serial numbers start with SM
	// USB PID is 0x1201, standard driver is DXX
	ICSNEO_FINDABLE_DEVICE(RADSupermoon, DeviceType::RADSupermoon, "SM");

	enum class SKU {
		Standard,
		APM1000ET, // Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::ETHERNET_01,
			Network::NetID::AE_01,
			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
				return SKU::APM1000ET;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::APM1000ET:
				return "Keysight APM1000ET";
		}
		return Device::getProductName();
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	ProductID getProductID() const override {
		return ProductID::RADSupermoon;
	}
protected:
	RADSupermoon(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADSupermoonSettings>(makeDriver);
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
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

	bool requiresVehiclePower() const override { return false; }

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