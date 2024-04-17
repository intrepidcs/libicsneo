#ifndef __VALUECAN3_H_
#define __VALUECAN3_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/valuecan3/valuecan3settings.h"

namespace icsneo {

class ValueCAN3 : public Device {
public:
	// USB PID is 0x0601, standard driver is FTDI
	ICSNEO_FINDABLE_DEVICE_BY_PID(ValueCAN3, DeviceType::VCAN3, 0x0601);

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::MSCAN
		};
		return supportedNetworks;
	}

	bool supportsComponentVersions() const override { return true; }

private:
	ValueCAN3(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<ValueCAN3Settings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	bool requiresVehiclePower() const override { return false; }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 512*2048;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif // __cplusplus

#endif