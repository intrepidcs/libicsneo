#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/a2bpacket.h"
#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>

using namespace icsneo;

class A2BEncoderDecoderTest : public ::testing::Test {
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

	std::vector<uint8_t> testBytes =
		{0xaa, 0x0c, 0x15, 0x00, 0x0b, 0x02, 0x00, 0x00,
		0x08, 0x00, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00,
		0x08, 0x04, 0x00, 0x00};

	std::vector<uint8_t> recvBytes =
		{0xaa, 0x00, 0x2a, 0x00, 0x0a, 0x02, 0x02, 0x01,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x03, 0x02, 0x00, 0x00, 0x08, 0x04,
		0x00, 0x00};
};

TEST_F(A2BEncoderDecoderTest, PacketEncoderTest)
{
	std::vector<uint8_t> bytestream;
	auto messagePtr = std::make_shared<icsneo::A2BMessage>((uint8_t)2, true, 8);
	messagePtr->network = icsneo::Network::NetID::A2B2;
	A2BMessage& message = *messagePtr.get();

	message[0][0] = (0x02 << 8) | (0x03);
	message[0][2] = (0x04 << 8) | (0x08);

	packetEncoder->encode(*packetizer, bytestream, messagePtr);
	EXPECT_EQ(bytestream, testBytes);
}

TEST_F(A2BEncoderDecoderTest, PacketDecoderTest)
{
	std::shared_ptr<icsneo::Message> decodeMsg;
	std::shared_ptr<icsneo::A2BMessage> message = std::make_shared<icsneo::A2BMessage>((uint8_t)2, true, 8);

	message->network = icsneo::Network::NetID::A2B1;
	message->setTxMsgBit(false);
	message->setMonitorBit(true);

	EXPECT_TRUE(message->setSample(0, 0, (0x02 << 8) | (0x03)));
	EXPECT_TRUE(message->setSample(2, 0, (0x04 << 8) | (0x08)));

	EXPECT_TRUE(packetizer->input(recvBytes));
	auto packets = packetizer->output();
	if(packets.empty()) {
		EXPECT_TRUE(false);
	}
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::A2BMessage>(decodeMsg);
	EXPECT_EQ(message->network, testMessage->network);
	EXPECT_EQ(message->data, testMessage->data);
	EXPECT_EQ(message->getNumChannels(), testMessage->getNumChannels());
	EXPECT_EQ(message->isMonitorMsg(), testMessage->isMonitorMsg());
	EXPECT_EQ(message->isTxMsg(), testMessage->isTxMsg());
	EXPECT_EQ(message->isErrIndicator(), testMessage->isErrIndicator());
	EXPECT_EQ(message->isSyncFrame(), testMessage->isSyncFrame());
	EXPECT_EQ(message->getRFU2(), testMessage->getRFU2());
}
