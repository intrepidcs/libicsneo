#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/mdiopacket.h"
#include "icsneo/communication/message/mdiomessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/core/ringbuffer.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>

using namespace icsneo;

class MDIOEncoderDecoderTest : public ::testing::Test {
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
	}
	device_eventhandler_t report;
	std::optional<Encoder> packetEncoder;
	std::optional<Packetizer> packetizer;
	std::optional<Decoder> packetDecoder;
	RingBuffer ringBuffer = RingBuffer(128);

	std::vector<uint8_t> testBytesClause22 =
	{0xAA, 0x0C, 0x11, 0x00, 0x21, 0x02, 0xAB, 0xCD,
	 0x01, 0x01, 0x18, 0x00, 0x14, 0x00, 0x56, 0x78};
	
	std::vector<uint8_t> testBytesClause45 =
	{0xAA, 0x0C, 0x11, 0x00, 0x21, 0x02, 0xAB, 0xCD,
	 0x02, 0x00, 0x06, 0x14, 0x34, 0x12, 0x56, 0x78};

	std::vector<uint8_t> testBytesClause22Mask =
	{0xAA, 0x0C, 0x11, 0x00, 0x21, 0x02, 0xFF, 0xFF,
	 0x01, 0x01, 0x1F, 0x00, 0x1F, 0x00, 0xFF, 0xFF};
	
	std::vector<uint8_t> testBytesClause45Mask =
	{0xAA, 0x0C, 0x11, 0x00, 0x21, 0x02, 0xFF, 0xFF,
	 0x02, 0x00, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF};

	std::vector<uint8_t> recvBytesClause22 =
		{
			0xaa, 0x0c,										// header
			0x22, 0x00,										// length
			0x21, 0x02,										// hw netid
			0x26, 0x0D,										// word1
			0x00, 0x00,										// word2
			0x14, 0x00,										// word3
			0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // data[8]
			0xCD, 0xAB,										// stats
			0x97, 0x29, 0xe6, 0xfb, 0xc1, 0xfc, 0xb0, 0x80, // timestamp
			0x4A, 0x00,										// netid
			0x00, 0x00,										// length
		};
	std::vector<uint8_t> recvBytesClause45 =
		{
			0xaa, 0x0c,										// header
			0x22, 0x00,										// length
			0x21, 0x02,										// hw netid
			0x92, 0x1C,										// word1
			0xFF, 0x00,										// word2
			0x56, 0x14,										// word3
			0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // data[8]
			0xCD, 0xAB,										// stats
			0x97, 0x29, 0xe6, 0xfb, 0xc1, 0xfc, 0xb0, 0x80, // timestamp
			0x4A, 0x00,										// netid
			0x00, 0x00,										// length
		};
};

TEST_F(MDIOEncoderDecoderTest, PacketEncoderClause22Test) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::MDIOMessage>();
	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xABCD;
	message->phyAddress = 0x18u;
	message->devAddress = 0x13u;
	message->regAddress = 0x14u;
	message->data = {0x56u, 0x78u};
	message->direction = MDIOMessage::Direction::Write;
	message->clause = MDIOMessage::Clause::Clause22;
	message->isTXMsg = true;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testBytesClause22);
}

TEST_F(MDIOEncoderDecoderTest, PacketEncoderClause45Test) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::MDIOMessage>();
	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xABCD;
	message->phyAddress = 0x06u;
	message->devAddress = 0x14u;
	message->regAddress = 0x1234u;
	message->data = {0x56u, 0x78u};
	message->direction = MDIOMessage::Direction::Read;
	message->clause = MDIOMessage::Clause::Clause45;
	message->isTXMsg = true;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testBytesClause45);
}

TEST_F(MDIOEncoderDecoderTest, PacketEncoderClause22MaskTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::MDIOMessage>();
	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xFFFFu;
	message->phyAddress = 0xFFu;
	message->devAddress = 0xFFu;
	message->regAddress = 0xFFFFu;
	message->data = {0xFFu, 0xFFu};
	message->direction = MDIOMessage::Direction::Write;
	message->clause = MDIOMessage::Clause::Clause22;
	message->isTXMsg = true;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testBytesClause22Mask);
}

TEST_F(MDIOEncoderDecoderTest, PacketEncoderClause45MaskTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::MDIOMessage>();
	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xFFFFu;
	message->phyAddress = 0xFFu;
	message->devAddress = 0xFFu;
	message->regAddress = 0xFFFFu;
	message->data = {0xFFu, 0xFFu};
	message->direction = MDIOMessage::Direction::Read;
	message->clause = MDIOMessage::Clause::Clause45;
	message->isTXMsg = true;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testBytesClause45Mask);
}

TEST_F(MDIOEncoderDecoderTest, PacketDecoderClause22Test) {
	std::shared_ptr<icsneo::Message> decodeMsg;
	std::shared_ptr<icsneo::MDIOMessage> message = std::make_shared<icsneo::MDIOMessage>();

	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xABCD;
	message->phyAddress = 0x06u;
	message->devAddress = 0x00u;
	message->regAddress = 0x14u;
	message->data = {0x56u, 0x78u};
	message->direction = MDIOMessage::Direction::Read;
	message->clause = MDIOMessage::Clause::Clause22;
	message->isTXMsg = true;
	message->timestamp = static_cast<uint64_t>(0xB0FCC1FBE62997);

	ringBuffer.clear();
	ringBuffer.write(recvBytesClause22);
	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();
	EXPECT_FALSE(packets.empty());
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::MDIOMessage>(decodeMsg);
	EXPECT_EQ(message->network, testMessage->network);
	EXPECT_EQ(message->description, testMessage->description);
	EXPECT_EQ(message->phyAddress, testMessage->phyAddress);
	EXPECT_EQ(message->devAddress, testMessage->devAddress);
	EXPECT_EQ(message->regAddress, testMessage->regAddress);
	EXPECT_EQ(message->data, testMessage->data);
	EXPECT_EQ(message->direction, testMessage->direction);
	EXPECT_EQ(message->clause, testMessage->clause);
	EXPECT_EQ(message->isTXMsg, testMessage->isTXMsg);
	EXPECT_EQ(message->txTimeout, testMessage->txTimeout);
	EXPECT_EQ(message->txAborted, testMessage->txAborted);
	EXPECT_EQ(message->txInvalidBus, testMessage->txInvalidBus);
	EXPECT_EQ(message->txInvalidPhyAddr, testMessage->txInvalidPhyAddr);
	EXPECT_EQ(message->txInvalidRegAddr, testMessage->txInvalidRegAddr);
	EXPECT_EQ(message->txInvalidClause, testMessage->txInvalidClause);
	EXPECT_EQ(message->txInvalidOpcode, testMessage->txInvalidOpcode);
	EXPECT_EQ(message->timestamp, testMessage->timestamp);
}

TEST_F(MDIOEncoderDecoderTest, PacketDecoderClause45Test) {
	std::shared_ptr<icsneo::Message> decodeMsg;
	std::shared_ptr<icsneo::MDIOMessage> message = std::make_shared<icsneo::MDIOMessage>();

	message->network = icsneo::Network::NetID::MDIO_01;
	message->description = 0xABCD;
	message->phyAddress = 0x12u;
	message->devAddress = 0x03u;
	message->regAddress = 0x1456u;
	message->data = {0x56u, 0x78u};
	message->direction = MDIOMessage::Direction::Write;
	message->clause = MDIOMessage::Clause::Clause45;
	message->isTXMsg = true;
	message->txTimeout = true;
	message->txAborted = true;
	message->txInvalidBus = true;
	message->txInvalidPhyAddr = true;
	message->txInvalidRegAddr = true;
	message->txInvalidClause = true;
	message->txInvalidOpcode = true;
	message->timestamp = static_cast<uint64_t>(0xB0FCC1FBE62997);

	ringBuffer.clear();
	ringBuffer.write(recvBytesClause45);
	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();
	EXPECT_FALSE(packets.empty());
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::MDIOMessage>(decodeMsg);
	EXPECT_EQ(message->network, testMessage->network);
	EXPECT_EQ(message->description, testMessage->description);
	EXPECT_EQ(message->phyAddress, testMessage->phyAddress);
	EXPECT_EQ(message->devAddress, testMessage->devAddress);
	EXPECT_EQ(message->regAddress, testMessage->regAddress);
	EXPECT_EQ(message->data, testMessage->data);
	EXPECT_EQ(message->direction, testMessage->direction);
	EXPECT_EQ(message->clause, testMessage->clause);
	EXPECT_EQ(message->isTXMsg, testMessage->isTXMsg);
	EXPECT_EQ(message->txTimeout, testMessage->txTimeout);
	EXPECT_EQ(message->txAborted, testMessage->txAborted);
	EXPECT_EQ(message->txInvalidBus, testMessage->txInvalidBus);
	EXPECT_EQ(message->txInvalidPhyAddr, testMessage->txInvalidPhyAddr);
	EXPECT_EQ(message->txInvalidRegAddr, testMessage->txInvalidRegAddr);
	EXPECT_EQ(message->txInvalidClause, testMessage->txInvalidClause);
	EXPECT_EQ(message->txInvalidOpcode, testMessage->txInvalidOpcode);
	EXPECT_EQ(message->timestamp, testMessage->timestamp);
}
