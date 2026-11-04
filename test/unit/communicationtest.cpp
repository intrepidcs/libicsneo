#include "gtest/gtest.h"

#include <cstring>
#include <memory>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/driver.h"
#include "icsneo/device/tree/radmoon2/radmoon2zl.h"

using namespace icsneo;

namespace {

class ReopenTestDriver : public Driver {
public:
	explicit ReopenTestDriver(const device_eventhandler_t& handler) : Driver(handler) {}

	bool open() override { opened = true; return true; }
	bool isOpen() override { return opened; }
	bool close() override { opened = false; return true; }
	driver_finder_t getFinder() override { return [](std::vector<FoundDevice>&) {}; }

protected:
	bool writeInternal(const std::vector<uint8_t>&) override { return true; }

private:
	bool opened = false;
};

}

// The heartbeat marks the driver disconnected and the user closes and opens again. Drivers like Servd stop their
// threads while disconnected, so the reopened connection must not start disconnected.
TEST(Communication, OpenAfterDisconnectIsConnected) {
	FoundDevice found;
	std::memcpy(found.serial, "RN0001", 6);
	found.makeDriver = [](device_eventhandler_t handler, neodevice_t&) {
		return std::make_unique<ReopenTestDriver>(handler);
	};
	auto device = std::make_shared<RADMoon2ZL>(found);
	ASSERT_TRUE(device->com->open());

	device->com->driver->setIsDisconnected(true);
	ASSERT_TRUE(device->com->close());
	ASSERT_TRUE(device->com->open());
	EXPECT_FALSE(device->com->isDisconnected());
	EXPECT_TRUE(device->com->close());
}
