#ifndef __RADSUPERMOON_H_
#define __RADSUPERMOON_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radsupermoon/radsupermoonsettings.h"
#include "icsneo/platform/ftdi3.h"

namespace icsneo {

class RADSupermoon : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADSupermoon;
	static constexpr const uint16_t PRODUCT_ID = 0x1201;
	static constexpr const char* SERIAL_START = "SM";

	enum class SKU {
		Standard,
		APM1000ET, // Keysight Branding
	};

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI3::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADSupermoon(neodevice));

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::Ethernet,
			Network::NetID::OP_Ethernet1,
			Network::NetID::OP_Ethernet2
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

protected:
	RADSupermoon(neodevice_t neodevice) : Device(neodevice) {
		initialize<FTDI3, RADSupermoonSettings>();
		productId = PRODUCT_ID;
		getWritableNeoDevice().type = DEVICE_TYPE;
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

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }
};

}

#endif // __cplusplus

#endif