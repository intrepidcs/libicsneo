#include <gtest/gtest.h>

#include <cstring>

#include "icsneo/communication/driver.h"
#include "icsneo/core/macseccfg.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radcomet2/radcomet2.h"
#include "icsneo/device/tree/radcomet3/radcomet3.h"
#include "icsneo/device/tree/radgigastar2/radgigastar2.h"
#include "icsneo/device/tree/radmoon2/radmoon2.h"
#include "icsneo/device/tree/radmoon2/radmoon2zl.h"
#include "icsneo/device/tree/radmoon3/radmoon3.h"

using namespace icsneo;

namespace {

class MACsecTestDriver : public Driver {
public:
	MACsecTestDriver(const device_eventhandler_t& handler) : Driver(handler) {}

	bool open() override { return true; }
	bool isOpen() override { return true; }
	bool close() override { return true; }
	driver_finder_t getFinder() override {
		return [](std::vector<FoundDevice>&) {};
	}
};

FoundDevice makeFoundDevice(const char* serial) {
	FoundDevice found;
	std::memcpy(found.serial, serial, sizeof(found.serial) - 1);
	found.serial[sizeof(found.serial) - 1] = '\0';
	found.makeDriver = [](device_eventhandler_t handler, neodevice_t&) {
		return std::make_unique<MACsecTestDriver>(handler);
	};
	return found;
}

std::vector<Network::NetID> getMACsecNetIDs(const Device& device) {
	std::vector<Network::NetID> netIDs;
	for(const auto& network : device.getMACsecNetworks())
		netIDs.push_back(network.getNetID());
	return netIDs;
}

TEST(MACsec, RADMoon2ZLOnlyMapsAutomotiveEthernet) {
	RADMoon2 moon2(makeFoundDevice("RM0001"));
	RADMoon2ZL moon2zl(makeFoundDevice("RN0001"));

	EXPECT_TRUE(getMACsecNetIDs(moon2).empty());
	EXPECT_EQ(getMACsecNetIDs(moon2zl), std::vector<Network::NetID>({Network::NetID::AE_01}));
}

TEST(MACsec, RADMoon3MapsIndependentPHYs) {
	RADMoon3 moon3(makeFoundDevice("R30001"));

	EXPECT_EQ(
		getMACsecNetIDs(moon3),
		std::vector<Network::NetID>({Network::NetID::AE_01, Network::NetID::ETHERNET_01})
	);
}

TEST(MACsec, RADGigastar2MapsBothMACsecPorts) {
	RADGigastar2 gigastar2(makeFoundDevice("GT0001"));

	EXPECT_EQ(
		getMACsecNetIDs(gigastar2),
		std::vector<Network::NetID>({Network::NetID::AE_01, Network::NetID::AE_02})
	);
}

TEST(MACsec, RADCometsMapOneMACsecPort) {
	RADComet2 comet2(makeFoundDevice("RC0300"));
	RADComet3 comet3(makeFoundDevice("C30001"));

	EXPECT_EQ(getMACsecNetIDs(comet2), std::vector<Network::NetID>({Network::NetID::AE_01}));
	EXPECT_EQ(getMACsecNetIDs(comet3), std::vector<Network::NetID>({Network::NetID::AE_01}));
	EXPECT_TRUE(MACsecConfig(comet2.getType()));
	EXPECT_TRUE(MACsecConfig(comet3.getType()));
}

} // namespace
