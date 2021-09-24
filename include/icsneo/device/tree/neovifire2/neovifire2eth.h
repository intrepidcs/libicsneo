#ifndef __NEOVIFIRE2ETH_H_
#define __NEOVIFIRE2ETH_H_

#ifdef __cplusplus

#include "icsneo/device/tree/neovifire2/neovifire2.h"
#include "icsneo/platform/pcap.h"
#include <memory>

namespace icsneo {

class NeoVIFIRE2ETH : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x0004;
	static std::vector<std::shared_ptr<Device>> Find(const std::vector<PCAP::PCAPFoundDevice>& pcapDevices) {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : pcapDevices) {
			auto fakedev = std::shared_ptr<NeoVIFIRE2ETH>(new NeoVIFIRE2ETH({}));
			for (auto& payload : foundDev.discoveryPackets)
				fakedev->com->packetizer->input(payload);
			for (auto& packet : fakedev->com->packetizer->output()) {
				std::shared_ptr<Message> msg;
				if (!fakedev->com->decoder->decode(msg, packet))
					continue; // We failed to decode this packet

				if(!msg || msg->type != Message::Type::Main51)
					continue; // Not a message we care about
				auto sn = std::dynamic_pointer_cast<SerialNumberMessage>(msg);
				if(!sn)
					continue; // Not a serial number message
				
				if(sn->deviceSerial.length() < 2)
					continue;
				if(sn->deviceSerial.substr(0, 2) != SERIAL_START)
					continue; // Not a FIRE 2
				
				auto device = foundDev.device;
				device.serial[sn->deviceSerial.copy(device.serial, sizeof(device.serial))] = '\0';
				found.push_back(std::make_shared<NeoVIFIRE2ETH>(std::move(device)));
				break;
			}
		}

		return found;
	}

	NeoVIFIRE2ETH(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		initialize<PCAP, NeoVIFIRE2Settings>();
		productId = PRODUCT_ID;
	}

	bool currentDriverSupportsDFU() const override { return false; }

protected:
	void setupSettings(IDeviceSettings& ssettings) override {
		// TODO Check firmware version, old firmwares will reset Ethernet settings on settings send
		ssettings.readonly = true;
	}

	void setupPacketizer(Packetizer& packetizer) override {
		NeoVIFIRE2::setupPacketizer(packetizer);
		packetizer.align16bit = false;
	}
};

}

#endif // __cplusplus

#endif