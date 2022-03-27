#ifndef __RADMOON2_H_
#define __RADMOON2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"

namespace icsneo {

class RADMoon2 : public Device {
public:
	// Serial numbers start with RM
	// USB PID is 0x1202, standard driver is FTDI3
	ICSNEO_FINDABLE_DEVICE(RADMoon2, DeviceType::RADMoon2, "RM");

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

protected:
	RADMoon2(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<RADMoon2Settings>(makeDriver);
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

	bool requiresVehiclePower() const override { return false; }
};

}

#endif // __cplusplus

#endif