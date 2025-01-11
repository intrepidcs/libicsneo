#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/message/apperrormessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/device/tree/neovired2/neovired2.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>
#include <iostream>

using namespace icsneo;

class REDAppErrorDecoderTest : public ::testing::Test {
protected:
	void SetUp() override {
		report = [](APIEvent::Type, APIEvent::Severity) {
			// Unless caught by the test, the packetizer should not throw errors
			EXPECT_TRUE(false);
		};

		packetizer.emplace([this](APIEvent::Type t, APIEvent::Severity s) { report(t, s); });
		packetEncoder.emplace([this](APIEvent::Type t, APIEvent::Severity s) { report(t, s); });
		packetDecoder.emplace([this](APIEvent::Type t, APIEvent::Severity s) { report(t, s); });
	}

	device_eventhandler_t report;
	std::optional<Encoder> packetEncoder;
	std::optional<Packetizer> packetizer;
	std::optional<Decoder> packetDecoder;
	
	RingBuffer ringBuffer = RingBuffer(128);

	std::vector<uint8_t> testErrorData =
	{0xaa, 0x0c,
	 0x12, 0x00, //size
	 0x34, 0x00, //netID
	 0x0b, 0x00, //error_type
	 0x01, 0x00, //network_id
	 0x33, 0x44, 0x55, 0x66, //uiTime10us
	 0x77, 0x88, 0x99, 0xAA, //uiTime10usMSB
	};
};

TEST_F(REDAppErrorDecoderTest, PacketDecoderTest) {
	std::shared_ptr<icsneo::Message> decodeMsg;

	auto msg1 = std::make_shared<icsneo::AppErrorMessage>();
	msg1->errorType = static_cast<uint16_t>(AppErrorType::AppErrorNetworkNotEnabled);
	msg1->errorNetID = Network::NetID::HSCAN;
	msg1->timestamp10us = 0x66554433;
	msg1->timestamp10usMSB = 0xAA998877;
	msg1->network = icsneo::Network::NetID::RED_App_Error;

	ringBuffer.clear();
	ringBuffer.write(testErrorData);

	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();

	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	EXPECT_NE(decodeMsg, nullptr);
	auto testMessage = std::dynamic_pointer_cast<icsneo::AppErrorMessage>(decodeMsg);
	EXPECT_EQ(msg1->network, testMessage->network);
	EXPECT_EQ(msg1->errorType, testMessage->errorType);
	EXPECT_EQ(msg1->errorNetID, testMessage->errorNetID);
	EXPECT_EQ(msg1->timestamp10us, testMessage->timestamp10us);
	EXPECT_EQ(msg1->timestamp10usMSB, testMessage->timestamp10usMSB);
	packets.pop_back();
}

TEST_F(REDAppErrorDecoderTest, GetErrorStringTest) {
	std::shared_ptr<icsneo::Message> decodeMsg;

	ringBuffer.clear();
	ringBuffer.write(testErrorData);

	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();

	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	EXPECT_NE(decodeMsg, nullptr);
	auto testMessage = std::dynamic_pointer_cast<icsneo::AppErrorMessage>(decodeMsg);
	EXPECT_EQ("HSCAN: Network not enabled", testMessage->getAppErrorString());
	packets.pop_back();
}

TEST_F(REDAppErrorDecoderTest, GetTypeTest) {
	std::shared_ptr<icsneo::Message> decodeMsg;

	ringBuffer.clear();
	ringBuffer.write(testErrorData);

	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();

	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	EXPECT_NE(decodeMsg, nullptr);
	auto testMessage = std::dynamic_pointer_cast<icsneo::AppErrorMessage>(decodeMsg);
	EXPECT_EQ(AppErrorType::AppErrorNetworkNotEnabled, testMessage->getAppErrorType());
	packets.pop_back();
}
