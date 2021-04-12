#ifndef __RADMOON2_H_
#define __RADMOON2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"
#include "icsneo/platform/ftdi3.h"

namespace icsneo {

class RADMoon2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADMoon2;
	static constexpr const uint16_t PRODUCT_ID = 0x1202;
	static constexpr const char* SERIAL_START = "RM";

	enum class SKU {
		Standard,
		APM1000E, // Keysight Branding
		APM1000E_CLK, // Clock Option and Keysight Branding
	};

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI3::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADMoon2(neodevice));

		return found;
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

protected:
	RADMoon2(neodevice_t neodevice) : Device(neodevice) {
		initialize<FTDI3, RADMoon2Settings>();
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
};

}

#endif // __cplusplus

#endif