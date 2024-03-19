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
		0x08, 0x00, 0x00, 0x00, 0xCC, 0xFF, 0x00, 0x00,
		0x9A, 0xFF, 0x00, 0x00};

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
	auto messagePtr = std::make_shared<icsneo::A2BMessage>(
		static_cast<size_t>(1u),
		icsneo::A2BMessage::TDMMode::TDM2,
		true
	);

	messagePtr->network = icsneo::Network::NetID::A2B2;
	A2BMessage& message = *messagePtr.get();

	message.setChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(0u),
		0u,
		-52,
		icsneo::PCMType::L16
	);

	message.setChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(1u),
		0u,
		-102,
		icsneo::PCMType::L16
	);

	packetEncoder->encode(*packetizer, bytestream, messagePtr);
	EXPECT_EQ(bytestream, testBytes);

	message.setChannelSample(
		icsneo::A2BMessage::Direction::Upstream,
		static_cast<uint8_t>(1u),
		0u,
		-102,
		icsneo::PCMType::L16
	);

	EXPECT_EQ(message.getChannelSample(
		icsneo::A2BMessage::Direction::Upstream,
		static_cast<uint8_t>(1u),
		0u,
		icsneo::PCMType::L16
	), -102);
}

TEST_F(A2BEncoderDecoderTest, PacketDecoderTest)
{
	std::shared_ptr<icsneo::Message> decodeMsg;
	auto message = std::make_shared<icsneo::A2BMessage>(
		static_cast<size_t>(1u),
		icsneo::A2BMessage::TDMMode::TDM2,
		true
	);

	message->network = icsneo::Network::NetID::A2B1;
	message->txmsg = false;
	message->monitor = true;

	message->setChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(0u),
		0u,
		(0x02 << 8) | (0x03),
		icsneo::PCMType::L16
	);

	message->setChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(1u),
		0u,
		(0x04 << 8) | (0x08),
		icsneo::PCMType::L16
	);

	EXPECT_TRUE(message->getChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(0u),
		0u,
		icsneo::PCMType::L16		
	) == static_cast<icsneo::PCMSample>((0x02 << 8) | (0x03)));

	EXPECT_TRUE(message->getChannelSample(
		icsneo::A2BMessage::Direction::Downstream,
		static_cast<uint8_t>(1u),
		0u,
		icsneo::PCMType::L16		
	) == static_cast<icsneo::PCMSample>((0x04 << 8) | (0x08)));

	EXPECT_TRUE(packetizer->input(recvBytes));
	auto packets = packetizer->output();
	if(packets.empty()) {
		EXPECT_TRUE(false);
	}
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::A2BMessage>(decodeMsg);
	EXPECT_EQ(message->network, testMessage->network);
	EXPECT_EQ(message->data, testMessage->data);
	EXPECT_EQ(message->numChannels, testMessage->numChannels);
	EXPECT_EQ(message->monitor, testMessage->monitor);
	EXPECT_EQ(message->txmsg, testMessage->txmsg);
	EXPECT_EQ(message->errIndicator, testMessage->errIndicator);
	EXPECT_EQ(message->syncFrame, testMessage->syncFrame);
	EXPECT_EQ(message->rfu2, testMessage->rfu2);
}
