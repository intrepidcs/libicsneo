#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::FIRE2;
	static constexpr const char* SERIAL_START = "CY";
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

protected:
	static std::shared_ptr<Communication> MakeCommunication(std::unique_ptr<ICommunication> transport) {
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		encoder->supportCANFD = true;
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		return std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
	}
};

}

#endif