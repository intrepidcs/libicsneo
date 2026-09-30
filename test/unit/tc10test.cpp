#include "gtest/gtest.h"

#include <cstring>
#include <memory>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/driver.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/message/filter/extendedresponsefilter.h"
#include "icsneo/communication/message/tc10statusmessage.h"
#include "icsneo/device/tree/radmoon2/radmoon2.h"
#include "icsneo/device/tree/radmoon2/radmoon2zl.h"

using namespace icsneo;

namespace {

class TC10TestDriver : public Driver {
public:
	explicit TC10TestDriver(const device_eventhandler_t& handler) : Driver(handler) {}

	bool open() override { opened = true; return true; }
	bool isOpen() override { return opened; }
	bool close() override { opened = false; return true; }
	driver_finder_t getFinder() override { return [](std::vector<FoundDevice>&) {}; }

	int writes = 0;
	// Replies pushed, in order, when the host writes a command.
	std::vector<std::vector<uint8_t>> replies;

protected:
	bool writeInternal(const std::vector<uint8_t>&) override {
		++writes;
		for(const auto& reply : replies)
			pushRx(reply.data(), reply.size());
		replies.clear();
		return true;
	}

private:
	bool opened = false;
};

std::vector<uint8_t> longPacket(Network::NetID netid, const void* payload, size_t payloadSize) {
	const uint16_t packetLength = static_cast<uint16_t>(6 + payloadSize);
	std::vector<uint8_t> bytes(packetLength);
	bytes[0] = 0xAA;
	bytes[1] = 0x00;
	bytes[2] = static_cast<uint8_t>(packetLength & 0xff);
	bytes[3] = static_cast<uint8_t>((packetLength >> 8) & 0xff);
	const auto id = static_cast<uint16_t>(netid);
	bytes[4] = static_cast<uint8_t>(id & 0xff);
	bytes[5] = static_cast<uint8_t>((id >> 8) & 0xff);
	std::memcpy(bytes.data() + 6, payload, payloadSize);
	return bytes;
}

std::vector<uint8_t> genericReturn(ExtendedCommand command, ExtendedResponse code) {
	ExtendedResponseMessage::PackedGenericResponse packed{};
	packed.header.command = ExtendedCommand::GenericReturn;
	packed.header.length = static_cast<uint16_t>(sizeof(packed) - sizeof(packed.header));
	packed.command = command;
	packed.returnCode = code;
	return longPacket(Network::NetID::ExtendedCommand, &packed, sizeof(packed));
}

// Extended body of a GetTC10Status reply. `tail` is extra payload past the two status bytes.
std::vector<uint8_t> statusBody(uint8_t wake, uint8_t sleep, std::initializer_list<uint8_t> tail = {}) {
	const auto payloadLength = static_cast<uint16_t>(2 + tail.size());
	std::vector<uint8_t> body = {
		0x3f, 0x00,
		static_cast<uint8_t>(payloadLength & 0xff),
		static_cast<uint8_t>((payloadLength >> 8) & 0xff),
		wake,
		sleep,
	};
	body.insert(body.end(), tail);
	return body;
}

class OpenTC10Device {
public:
	TC10TestDriver* driver = nullptr;
	std::shared_ptr<RADMoon2ZL> device;

	OpenTC10Device() {
		FoundDevice found;
		std::memcpy(found.serial, "RN0001", 6);
		found.makeDriver = [this](device_eventhandler_t handler, neodevice_t&) {
			auto result = std::make_unique<TC10TestDriver>(handler);
			driver = result.get();
			return result;
		};
		device = std::make_shared<RADMoon2ZL>(found);
		device->com->readTaskWakeTimeout = std::chrono::milliseconds(20);
		EXPECT_TRUE(device->com->open());
	}
};

FoundDevice moon2Found() {
	FoundDevice found;
	std::memcpy(found.serial, "RM0001", 6);
	found.makeDriver = [](device_eventhandler_t handler, neodevice_t&) {
		return std::make_unique<TC10TestDriver>(handler);
	};
	return found;
}

}

TEST(TC10, WakeFilterMatchesOnlyThatCommand) {
	ExtendedResponseFilter filter(ExtendedCommand::RequestTC10Wake);
	EXPECT_TRUE(filter.match(std::make_shared<ExtendedResponseMessage>(
		ExtendedCommand::RequestTC10Wake, ExtendedResponse::OperationFailed)));
	EXPECT_FALSE(filter.match(std::make_shared<ExtendedResponseMessage>(
		ExtendedCommand::RequestTC10Sleep, ExtendedResponse::OK)));
	EXPECT_FALSE(filter.match(std::make_shared<ExtendedResponseMessage>(
		ExtendedCommand::GetDiskDetails, ExtendedResponse::OK)));
}

TEST(TC10, SleepFilterMatchesOnlyThatCommand) {
	ExtendedResponseFilter filter(ExtendedCommand::RequestTC10Sleep);
	EXPECT_TRUE(filter.match(std::make_shared<ExtendedResponseMessage>(
		ExtendedCommand::RequestTC10Sleep, ExtendedResponse::OK)));
	EXPECT_FALSE(filter.match(std::make_shared<ExtendedResponseMessage>(
		ExtendedCommand::RequestTC10Wake, ExtendedResponse::OK)));
}

TEST(TC10, UnsupportedDeviceReportsNotSupported) {
	auto device = std::make_shared<RADMoon2>(moon2Found());
	EXPECT_FALSE(device->requestTC10Wake(Network::NetID::AE_01));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NotSupported);
	EXPECT_FALSE(device->requestTC10Sleep(Network::NetID::AE_01));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NotSupported);
	EXPECT_FALSE(device->getTC10Status(Network::NetID::AE_01).has_value());
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NotSupported);
}

TEST(TC10, WakeAcceptsItsOwnOk) {
	OpenTC10Device tc10;
	(void)GetLastError();
	tc10.driver->replies = {
		genericReturn(ExtendedCommand::RequestTC10Sleep, ExtendedResponse::OperationFailed),
		genericReturn(ExtendedCommand::RequestTC10Wake, ExtendedResponse::OK),
	};
	EXPECT_TRUE(tc10.device->requestTC10Wake(Network::NetID::AE_02));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);
	EXPECT_EQ(tc10.driver->writes, 1);
}

TEST(TC10, SleepAcceptsItsOwnOk) {
	OpenTC10Device tc10;
	(void)GetLastError();
	tc10.driver->replies = {
		genericReturn(ExtendedCommand::RequestTC10Wake, ExtendedResponse::OperationFailed),
		genericReturn(ExtendedCommand::RequestTC10Sleep, ExtendedResponse::OK),
	};
	EXPECT_TRUE(tc10.device->requestTC10Sleep(Network::NetID::AE_01));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);
	EXPECT_EQ(tc10.driver->writes, 1);
}

TEST(TC10, WakeReportsDeviceRejection) {
	OpenTC10Device tc10;
	tc10.driver->replies = {
		genericReturn(ExtendedCommand::RequestTC10Sleep, ExtendedResponse::OK),
		genericReturn(ExtendedCommand::RequestTC10Wake, ExtendedResponse::OperationFailed),
	};
	EXPECT_FALSE(tc10.device->requestTC10Wake(Network::NetID::AE_02));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::TC10RequestFailed);
	EXPECT_EQ(tc10.driver->writes, 1);
}

TEST(TC10, SleepReportsDeviceRejection) {
	OpenTC10Device tc10;
	tc10.driver->replies = {
		genericReturn(ExtendedCommand::RequestTC10Wake, ExtendedResponse::OK),
		genericReturn(ExtendedCommand::RequestTC10Sleep, ExtendedResponse::InvalidParameter),
	};
	EXPECT_FALSE(tc10.device->requestTC10Sleep(Network::NetID::AE_01));
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::TC10RequestFailed);
	EXPECT_EQ(tc10.driver->writes, 1);
}

TEST(TC10Status, DecodesTheStatusPair) {
	auto msg = TC10StatusMessage::DecodeToMessage(statusBody(1, 1));
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->wakeStatus, TC10WakeStatus::WakeReceived);
	EXPECT_EQ(msg->sleepStatus, TC10SleepStatus::SleepReceived);
}

TEST(TC10Status, RejectsTruncatedOrForeignPayloads) {
	EXPECT_EQ(TC10StatusMessage::DecodeToMessage({}), nullptr);
	auto bytes = statusBody(0, 0);
	EXPECT_EQ(TC10StatusMessage::DecodeToMessage({bytes.begin(), bytes.begin() + 5}), nullptr);
	std::vector<uint8_t> shortLength = {0x3f, 0x00, 0x01, 0x00, 0x00, 0x00};
	EXPECT_EQ(TC10StatusMessage::DecodeToMessage(shortLength), nullptr);
	std::vector<uint8_t> foreign = {0x3d, 0x00, 0x02, 0x00, 0x00, 0x00};
	EXPECT_EQ(TC10StatusMessage::DecodeToMessage(foreign), nullptr);
}

TEST(TC10Status, DropsBytesAfterTheStatusPair) {
	// Tail is little-endian AE_02 (37). The decoded message has no network, so this still succeeds.
	auto msg = TC10StatusMessage::DecodeToMessage(statusBody(0, 1, {37, 0}));
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->wakeStatus, TC10WakeStatus::NoWakeReceived);
	EXPECT_EQ(msg->sleepStatus, TC10SleepStatus::SleepReceived);
}

TEST(TC10, StatusReturnsTheDecodedPair) {
	OpenTC10Device tc10;
	(void)GetLastError();
	const auto body = statusBody(1, 0);
	tc10.driver->replies = {longPacket(Network::NetID::ExtendedCommand, body.data(), body.size())};
	const auto status = tc10.device->getTC10Status(Network::NetID::AE_01);
	ASSERT_TRUE(status.has_value());
	EXPECT_EQ(status->wakeStatus, TC10WakeStatus::WakeReceived);
	EXPECT_EQ(status->sleepStatus, TC10SleepStatus::NoSleepReceived);
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);
	EXPECT_EQ(tc10.driver->writes, 1);
}
