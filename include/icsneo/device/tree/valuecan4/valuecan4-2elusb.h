#ifndef __VALUECAN4_2EL_USB_H_
#define __VALUECAN4_2EL_USB_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4-2el.h"
#include "icsneo/platform/cdcacm.h"
#include <string>

namespace icsneo {

class ValueCAN4_2EL_USB : public ValueCAN4_2EL {
public:
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : CDCACM::FindByProduct(USB_PRODUCT_ID)) {
			if(std::string(neodevice.serial).substr(0, 2) == SERIAL_START)
				found.emplace_back(new ValueCAN4_2EL_USB(neodevice));
		}

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			Network::NetID::Ethernet
		};
		return supportedNetworks;
	}

protected:
	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

private:
	ValueCAN4_2EL_USB(neodevice_t neodevice) : ValueCAN4_2EL(neodevice) {
		initialize<CDCACM, ValueCAN4_2ELSettings>();
		productId = USB_PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif