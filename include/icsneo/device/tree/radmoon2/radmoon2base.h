#ifndef __RADMOON2BASE_H_
#define __RADMOON2BASE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class RADMoon2Base : public Device {
public:
	enum class SKU {
		Standard,
		APM1000E, // Keysight Branding
		APM1000E_CLK, // Clock Option and Keysight Branding
	};

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::MDIO_01,
			Network::NetID::MDIO_02,
		};
		return supportedNetworks;
	}

	SKU getSKU() const {
		switch(getSerial().back()) {
			case 'A':
			case 'B':
				return SKU::APM1000E;
			case 'C':
			case 'D':
				return SKU::APM1000E_CLK;
			default:
				return SKU::Standard;
		}
	}

	std::string getProductName() const override {
		switch(getSKU()) {
			case SKU::Standard: break;
			case SKU::APM1000E:
				return "Keysight APM1000E";
			case SKU::APM1000E_CLK:
				return "Keysight APM1000E-CLK";
		}
		return Device::getProductName();
	}

	bool goOnline() override {
		if(!com->sendCommand(Command::EnableNetworkCommunication, true))
			return false;
		
		updateLEDState();
		online = true;
		return true;
	}

	bool getEthPhyRegControlSupported() const override { return true; }

	virtual uint8_t getPhyAddrOrPort() const = 0;

protected:
	using Device::Device;

	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportEthPhy = true;
	}

	bool requiresVehiclePower() const override { return false; }

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return std::nullopt;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return std::nullopt;
	}
};

}

#endif // __cplusplus

#endif