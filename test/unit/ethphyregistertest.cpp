#include "icsneo/communication/message/ethphymessage.h"
#include "icsneo/communication/packet/ethphyregpacket.h"
#include "gtest/gtest.h"
#include "icsneo/device/tree/radjupiter/radjupiter.h"
using namespace icsneo;
namespace {
const device_eventhandler_t ignore = [](APIEvent::Type, APIEvent::Severity) {};
EthPhyMessage request(bool clause45 = false, bool write = false) {
	EthPhyMessage msg;
	msg.appendPhyMessage(write, clause45, 31, clause45 ? 31 : 255, clause45 ? 65535 : 31, 0x1234);
	msg.messages[0]->BusIndex = 15;
	return msg;
}
std::vector<uint8_t> encode(const EthPhyMessage& msg) {
	std::vector<uint8_t> bytes;
	EXPECT_TRUE(HardwareEthernetPhyRegisterPacket::EncodeFromMessage(msg, bytes, ignore));
	return bytes;
}
}
TEST(EthPhyRegister, KnownWireBytes) {
	for(bool clause45 : {false, true}) {
		auto msg = request(clause45, true);
		auto bytes = encode(msg);
		EXPECT_EQ(bytes, (std::vector<uint8_t>{1, 0, 1, 8, static_cast<uint8_t>(clause45 ? 7 : 3), 0x1f,
			31, static_cast<uint8_t>(clause45 ? 31 : 255), static_cast<uint8_t>(clause45 ? 255 : 31),
			static_cast<uint8_t>(clause45 ? 255 : 0), 0x34, 0x12}));
	}
}
TEST(EthPhyRegister, InvalidRequestsLeaveOutputUntouched) {
	for(int invalid = 0; invalid < 8; ++invalid) {
		auto msg = request();
		switch(invalid) {
		case 0: msg.messages.clear(); break;
		case 1: msg.messages.resize(129, msg.messages[0]); break;
		case 2: msg.messages.push_back(nullptr); break;
		case 3: msg.messages[0]->BusIndex = 16; break;
		case 4: msg.messages[0]->Version = 0; break;
		case 5: msg.messages[0]->Clause22.phyAddr = 32; break;
		case 6: msg.messages[0]->Clause22.regAddr = 32; break;
		case 7: msg.messages = request(true).messages; msg.messages[0]->Clause45.device = 32; break;
		}
		std::vector<uint8_t> bytes{0xaa};
		EXPECT_FALSE(HardwareEthernetPhyRegisterPacket::EncodeFromMessage(msg, bytes, ignore));
		EXPECT_EQ(bytes, (std::vector<uint8_t>{0xaa}));
	}
	auto msg = request(); msg.messages.resize(128, msg.messages[0]);
	EXPECT_EQ(encode(msg).size(), 1028u);
}
TEST(EthPhyRegister, RejectMalformedResponses) {
	const auto valid = encode(request());
	for(size_t size = 0; size < valid.size(); ++size)
		EXPECT_EQ(HardwareEthernetPhyRegisterPacket::DecodeToMessage({valid.begin(), valid.begin() + size}, ignore), nullptr);
	for(size_t index : {0u, 2u, 3u, 5u}) {
		auto bytes = valid; bytes[index] = 0;
		EXPECT_EQ(HardwareEthernetPhyRegisterPacket::DecodeToMessage(bytes, ignore), nullptr);
	}
	auto extra = valid; extra.push_back(0);
	EXPECT_EQ(HardwareEthernetPhyRegisterPacket::DecodeToMessage(extra, ignore), nullptr);
}

namespace {
class PhyTestDriver : public Driver {
public:
	explicit PhyTestDriver(const device_eventhandler_t& report) : Driver(report) {}
	bool open() override { return false; }
	bool isOpen() override { return opened; }
	bool opened = true;
	bool close() override { opened = false; return true; }
	driver_finder_t getFinder() override { return [](std::vector<FoundDevice>&) {}; }
	int writes = 0;
protected:
	bool writeInternal(const std::vector<uint8_t>&) override { ++writes; return false; }
};
class PhyTestJupiter : public RADJupiter {
public:
	explicit PhyTestJupiter(const FoundDevice& found) : RADJupiter(found) {}
	~PhyTestJupiter() override { com->driver->close(); }
	bool isOpen() const override { return true; } // Exercise the native send path, without hardware.
};
}
TEST(EthPhyRegister, DeviceDoesNotSendInvalidRequests) {
	PhyTestDriver* driver = nullptr;
	FoundDevice found;
	found.makeDriver = [&driver](device_eventhandler_t report, neodevice_t&) {
		auto result = std::make_unique<PhyTestDriver>(report);
		driver = result.get();
		return result;
	};
	PhyTestJupiter device(found);
	auto msg = request();
	msg.messages.push_back(nullptr);
	EXPECT_FALSE(device.sendEthPhyMsg(msg).has_value());
	EXPECT_EQ(driver->writes, 0);
	msg.messages = request().messages; msg.messages[0]->BusIndex = 16;
	EXPECT_FALSE(device.sendEthPhyMsg(msg).has_value());
	EXPECT_EQ(driver->writes, 0);
	msg.messages = request().messages;
	EXPECT_FALSE(device.sendEthPhyMsg(msg, std::chrono::milliseconds(0)).has_value());
	EXPECT_EQ(driver->writes, 0);
	// The valid request reaches the fake driver's explicit send failure.
	EXPECT_FALSE(device.sendEthPhyMsg(msg).has_value());
	EXPECT_EQ(driver->writes, 1);
}
