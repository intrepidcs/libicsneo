#ifndef __NEOVIFIRE2ETH_H_
#define __NEOVIFIRE2ETH_H_

#include "device/neovifire2/include/neovifire2.h"
#include "platform/include/pcap.h"
#include "device/neovifire2/include/neovifire2settings.h"
#include <memory>

namespace icsneo {

class NeoVIFIRE2ETH : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x0004;
	NeoVIFIRE2ETH(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new PCAP(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
		settings = std::make_shared<NeoVIFIRE2Settings>(com);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : PCAP::FindByProduct(PRODUCT_ID)) {
			{ // Scope created so that we don't have two of the same device at once
				strncpy(neodevice.serial, SERIAL_FIND_ON_OPEN, sizeof(neodevice.serial));
				neodevice.serial[sizeof(neodevice.serial) - 1] = '\0';
				auto device = std::make_shared<NeoVIFIRE2ETH>(neodevice);
				if(!device->open()) // We will get the serial number on open
					continue; // If the open failed, we won't display the device as an option to connect to
				strncpy(neodevice.serial, device->getNeoDevice().serial, sizeof(neodevice.serial));
				neodevice.serial[sizeof(neodevice.serial) - 1] = '\0';
			}
			found.push_back(std::make_shared<NeoVIFIRE2ETH>(neodevice));
		}

		return found;
	}
};

}

#endif