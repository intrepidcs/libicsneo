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

	// RADMoon 2 does not go online, you can only set settings and
	// view PHY information (when supported)
	bool goOnline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
	}

	bool goOffline() override {
		report(APIEvent::Type::OnlineNotSupported, APIEvent::Severity::Error);
		return false;
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