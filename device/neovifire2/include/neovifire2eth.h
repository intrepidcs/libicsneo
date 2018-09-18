#ifndef __NEOVIFIRE2ETH_H_
#define __NEOVIFIRE2ETH_H_

#include "device/neovifire2/include/neovifire2.h"
#include "platform/include/pcap.h"

namespace icsneo {

class NeoVIFIRE2ETH : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t USB_PRODUCT_ID = 0x0004;
	NeoVIFIRE2ETH(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		com = std::make_shared<Communication>(std::make_shared<PCAP>(getWritableNeoDevice()));
		usbProductId = USB_PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : PCAP::FindByProduct(USB_PRODUCT_ID))
			found.push_back(std::make_shared<NeoVIFIRE2ETH>(neodevice));

		return found;
	}
};

};

#endif