#ifndef __NEOVIFIRE_H_
#define __NEOVIFIRE_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/tree/neovifire/neovifiresettings.h"

namespace icsneo {

class NeoVIFIRE : public Device {
public:
	// USB PID is 0x0701, standard driver is FTDI
	ICSNEO_FINDABLE_DEVICE_BY_PID(NeoVIFIRE, DeviceType::FIRE, 0x0701);

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::DWCAN_01,
			Network::NetID::DWCAN_08,
			Network::NetID::DWCAN_02,
			Network::NetID::DWCAN_03,

			Network::NetID::LSFTCAN_01,

			Network::NetID::SWCAN_01,

			Network::NetID::LIN_01,
			Network::NetID::LIN_02,
			Network::NetID::LIN_03,
			Network::NetID::LIN_04
		};
		return supportedNetworks;
	}

	enum class Mode : char {
		Application = 'A',
		Bootloader = 'B'
	};

	bool afterCommunicationOpen() override {
		if(!Device::afterCommunicationOpen()) // Doesn't do anything right now but just in case it gets added to later
			return false;

		// Enter mode is only needed on very old FIRE devices (white board), will be ignored by newer devices
		return enterMode(Mode::Application);
	}

	bool enterMode(Mode mode) {
		// Included for compatibility with bootloaders on very old FIRE devices (white board)
		// Mode will be a uppercase char like 'A'
		if(!com->rawWrite({ (uint8_t)mode }))
			return false;

		// We then expect to see that same mode back in lowercase
		// This won't happen in the case of new devices, though, so we assume it worked
		return true;
	}

private:
	NeoVIFIRE(neodevice_t neodevice, const driver_factory_t& makeDriver) : Device(neodevice) {
		initialize<NeoVIFIRESettings>(makeDriver);
	}

	void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

	std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override {
		return 4 * 512;
	}

	std::optional<MemoryAddress> getCoreminiStartAddressSD() const override {
		return 0;
	}
};

}

#endif // __cplusplus

#endif