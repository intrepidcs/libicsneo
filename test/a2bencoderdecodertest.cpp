#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/a2bpacket.h"
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
		packetizer.emplace([this](APIEvent::Type t, APIEvent::Severity s) {
			report(t, s);
		});
		packetEncoder.emplace([this](APIEvent::Type t, APIEvent::Severity s) {
			report(t, s);
		});
		packetDecoder.emplace([this](APIEvent::Type t, APIEvent::Severity s) {
			report(t, s);
		});

		A2BPCMSample initialSample = 10 << 8;
		constructTest(
			msg1,
			msg1Encoded,
			{1,1,0,0,0,0},
			1,
			true,
			false,
			false,
			false,
			false,
			0,
			3,
			initialSample
		);

		initialSample = (29 << 16) | (10 << 8);
		constructTest(
			msg2,
			msg2Encoded,
			{1,0,9,0,0,0},
			1,
			false,
			true,
			false,
			false,
			true,
			0,
			1,
			initialSample
		);

		initialSample = (29 << 16) | (10 << 8);
		constructTest(
			msg3,
			msg3Encoded,
			{2,0,6,0,0xAA,0xFF},
			2,
			false,
			false,
			true,
			true,
			false,
			0xFFAA,
			3,
			initialSample
		);

		constructTest(
			msg4,
			msg4Encoded,
			{4,0,6,0,0xAA,0xFF},
			4,
			false,
			false,
			true,
			true,
			false,
			0xFFAA,
			0,
			initialSample
		);
	}

	void constructTest(
		std::shared_ptr<A2BMessage>& testMsg,
		std::vector<uint8_t>& testMsgEncoded,
		std::array<uint8_t, 6> encodedHeader,
		uint8_t numChannels,
		bool channelSize16,
		bool monitor,
		bool txmsg,
		bool errIndicator,
		bool syncFrame,
		uint16_t rfu2,
		uint32_t numFrames,
		A2BPCMSample initialSample
	)
	{

		testMsg = std::make_shared<A2BMessage>(
			(uint8_t)(channelSize16 ? 16 : 24),
			(uint8_t)(channelSize16 ? 2 : 4),
			numChannels
		);

		auto addSample = [&](
			uint8_t channel,
			A2BMessage::A2BDirection dir
		) {
			testMsg->addSample(std::move(initialSample), dir, channel);
			
			for(size_t i = 0; i < static_cast<size_t>(testMsg->getBytesPerSample()); i++) {
				testMsgEncoded.push_back(static_cast<uint8_t>(initialSample >> (i * 8)));
			}

			initialSample++;
		};

		testMsg->network = Network(Network::NetID::A2B1);
		testMsg->channelSize16 = channelSize16;
		testMsg->monitor = monitor;
		testMsg->txmsg = txmsg;
		testMsg->errIndicator = errIndicator;
		testMsg->syncFrame = syncFrame;
		testMsg->rfu2 = rfu2;

		testMsgEncoded.reserve(
			HardwareA2BPacket::coreMiniMessageHeaderSize + numFrames * static_cast<size_t>(testMsg->getBytesPerSample())
		);

		testMsgEncoded.insert(
			testMsgEncoded.end(),
			encodedHeader.begin(),
			encodedHeader.end()
		); // Insert header
		appendCoreMiniHeaderOffset(testMsgEncoded);

		for(unsigned int frame = 0; frame < numFrames; frame++) {
			for(uint8_t channel = 0; channel < testMsg->getNumChannels(); channel++) {

				addSample(channel, A2BMessage::A2BDirection::DownStream);
				addSample(channel, A2BMessage::A2BDirection::UpStream);

			}
		}
	}

	void appendCoreMiniHeaderOffset(std::vector<uint8_t>& buf) {
		for(
			size_t i = 0;
			i < (HardwareA2BPacket::coreMiniMessageHeaderSize - HardwareA2BPacket::a2bHeaderSize); 
			i++
		) {
			buf.push_back(0);
		}
	}

	void decrementEncodedMessageSize(std::vector<uint8_t>& bytestream) {
		if(bytestream.size() < 6) 
			return;
		
		uint16_t size = (bytestream[3] << 8) | bytestream[2];

		if(size == 0)
			return;
		
		size--;

		bytestream[2] = (uint8_t)size;
		bytestream[3] = (uint8_t)(size >> 8);
	}

	void testMessage(std::shared_ptr<A2BMessage>& msg, std::vector<uint8_t>& msgEncoded) {
		std::vector<uint8_t> bytestream;
		std::vector<uint8_t> rawPacketBytes;
		std::shared_ptr<Message> decodeMsg;
		std::shared_ptr<A2BMessage> decodeA2BMsg;

		EXPECT_TRUE(packetEncoder->encode(*packetizer, bytestream, msg));
		rawPacketBytes = std::vector<uint8_t>(bytestream.begin() + 6, bytestream.end());

		EXPECT_EQ(rawPacketBytes, msgEncoded);
		decrementEncodedMessageSize(bytestream);

		EXPECT_TRUE(packetizer->input(bytestream));
		auto packets = packetizer->output();
		if(packets.empty()) { EXPECT_TRUE(false);}
		EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
		decodeA2BMsg = std::static_pointer_cast<A2BMessage>(decodeMsg);

		EXPECT_EQ(*msg, *decodeA2BMsg);
	}

	device_eventhandler_t report;
	std::optional<Encoder> packetEncoder;
	std::optional<Packetizer> packetizer;
	std::optional<Decoder> packetDecoder;

	std::shared_ptr<A2BMessage> msg1;
	std::vector<uint8_t> msg1Encoded;

	std::shared_ptr<A2BMessage> msg2;
	std::vector<uint8_t> msg2Encoded;

	std::shared_ptr<A2BMessage> msg3;
	std::vector<uint8_t> msg3Encoded;

	std::shared_ptr<A2BMessage> msg4;
	std::vector<uint8_t> msg4Encoded;
};

TEST_F(A2BEncoderDecoderTest, ChannelSize16SingleChannel) {
	testMessage(msg1, msg1Encoded);
}

TEST_F(A2BEncoderDecoderTest, ChannelSize24MultiChannel) {
	testMessage(msg2, msg2Encoded);
}

TEST_F(A2BEncoderDecoderTest, ChannelSize24MultiChannelMultiFrame) {
	testMessage(msg3, msg3Encoded);
}

TEST_F(A2BEncoderDecoderTest, NoFrames) {
	testMessage(msg4, msg4Encoded);
}
