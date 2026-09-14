#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/linpacket.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/message/transmitmessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/core/ringbuffer.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>
#include <iostream>
#include <cstring>

using namespace icsneo;

class LINEncoderDecoderTest : public ::testing::Test {
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
	//Responder load data before response LIN 2
	// ID 0x22 pID 0xE2 length 8
	std::vector<uint8_t> testRespData =
	{0xaa, 0x0c,
	 0x15, 0x00,
	 0x30, 0x00,
	 0x00, 0x0c,
	 0x00, 0x00,
	 0xe2,
	 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
	 0x99};

	//Controller header LIN 1
	// ID 0x22 pID 0xE2 length 8
	std::vector<uint8_t> testControllerHeaderOnly =
	{0xaa, 0x0c,
	 0x0d, 0x00,
	 0x10, 0x00,
	 0x00, 0x83,
	 0x00, 0x00,
	 0xE2, 0x41};

	// Break-only LIN 1: commander (0x80) | break-only (0x20) | len 3 => 0xa3
	// ID 0x00 pID 0x80
	std::vector<uint8_t> testControllerBreakOnly =
	{0xaa, 0x0c,
	 0x0d, 0x00,
	 0x10, 0x00,
	 0x00, 0xa3,
	 0x00, 0x00,
	 0x80, 0x41};

	std::vector<uint8_t> recvBytes = 
	{0xaa, 0x0c, 0x22, 0x00,
	 0x10, 0x00, 0x88, 0x03,
	 0x00, 0x08, 0x04, 0x00,
	 0xaa, 0xbb, 0xcc, 0xcc,
	 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0xb3, 0x34,
	 0xa8, 0x10, 0x29, 0x13,
	 0x48, 0x00, 0x02, 0x00,
	 0x00, 0x00,
	 0xaa, 0x0c, 0x22, 0x00,
	 0x30, 0x00, 0x88, 0x03,
	 0x00, 0x04, 0x04, 0x00,
	 0xaa, 0xbb, 0xcc, 0xcc,
	 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0xb4, 0x34,
	 0xa8, 0x10, 0x29, 0x13,
	 0x48, 0x00, 0x03, 0x00,
	 0x00, 0x00};

std::vector<uint8_t> testControllerWithData = 
	{0xaa, 0x0c,
	 0x11, 0x00,
	 0x10, 0x00,
	 0x00, 0x87,
	 0x00, 0x00,
	 0x11, 0xaa,
	 0xbb, 0xcc,
	 0xcc, 0x41};
};

TEST_F(LINEncoderDecoderTest, ProtectedIDCalcTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	message->network = icsneo::Network::NetID::LIN_01;
	message->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
	message->isEnhancedChecksum = false;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(message->protectedID, 0xE2);
}

TEST_F(LINEncoderDecoderTest, ChecksumCalcTestClassic) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	message->network = icsneo::Network::NetID::LIN_02;
	message->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
	message->isEnhancedChecksum = false;
	message->data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(message->checksum, 0x99);
}

TEST_F(LINEncoderDecoderTest, ChecksumCalcTestEnhanced) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	message->network = icsneo::Network::NetID::LIN_02;
	message->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
	message->isEnhancedChecksum = true;
	message->data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(message->checksum, 0xB6);
}

TEST_F(LINEncoderDecoderTest, PacketEncoderResponderLoadTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	message->network = icsneo::Network::NetID::LIN_02;
	message->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
	message->isEnhancedChecksum = false;
	message->data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testRespData);
}

TEST_F(LINEncoderDecoderTest, PacketEncoderControllerHeaderTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	message->network = icsneo::Network::NetID::LIN_01;
	message->linMsgType = icsneo::LINMessage::Type::LIN_HEADER_ONLY;
	message->isEnhancedChecksum = false;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testControllerHeaderOnly);
}

TEST_F(LINEncoderDecoderTest, PacketEncoderControllerBreakOnlyTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x00u));
	message->network = icsneo::Network::NetID::LIN_01;
	message->linMsgType = icsneo::LINMessage::Type::LIN_BREAK_ONLY;
	message->isEnhancedChecksum = false;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testControllerBreakOnly);
}

TEST_F(LINEncoderDecoderTest, TransmitMessageBreakOnlySetsCommander) {
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x00u));
	message->network = icsneo::Network::NetID::LIN_01;
	message->linMsgType = icsneo::LINMessage::Type::LIN_BREAK_ONLY;
	auto bytes = TransmitMessage::EncodeFromMessage(message, 1, report);
	ASSERT_FALSE(bytes.empty());
	ASSERT_GE(bytes.size(), sizeof(TransmitMessage));
	auto* tx = reinterpret_cast<TransmitMessage*>(bytes.data());
	auto* lin = reinterpret_cast<HardwareLINPacket*>(tx->commonHeader);
	EXPECT_EQ(lin->CoreMiniBitsLIN.TXCommander, 1);
	EXPECT_EQ(lin->CoreMiniBitsLIN.BreakOnly, 1);
}

TEST_F(LINEncoderDecoderTest, PacketEncoderControllerWithDataTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x11u));
	message->network = icsneo::Network::NetID::LIN_01;
	message->linMsgType = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
	message->isEnhancedChecksum = false;
	message->data = {0xaa, 0xbb, 0xcc};
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testControllerWithData);
}

TEST_F(LINEncoderDecoderTest, PacketDecoderTest) {
	std::shared_ptr<icsneo::Message> decodeMsg;

	auto msg1 = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	msg1->network = icsneo::Network::NetID::LIN_02;
	msg1->linMsgType = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
	msg1->isEnhancedChecksum = false;
	msg1->data = {0xaa, 0xbb, 0xcc};
	msg1->checksum = 0xcc;

	auto msg2 = std::make_shared<icsneo::LINMessage>(static_cast<uint8_t>(0x22u));
	msg2->network = icsneo::Network::NetID::LIN_01;
	msg2->linMsgType = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
	msg2->isEnhancedChecksum = false;
	msg2->data = {0xaa, 0xbb, 0xcc};
	msg2->checksum = 0xcc;

	ringBuffer.clear();
	ringBuffer.write(recvBytes);
	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();
	if(packets.size() != 2) { EXPECT_TRUE(false); }
	//LIN_02 frame from device
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::LINMessage>(decodeMsg);
	EXPECT_EQ(msg1->network, testMessage->network);
	EXPECT_EQ(msg1->ID, testMessage->ID);
	EXPECT_EQ(msg1->type, testMessage->type);
	EXPECT_EQ(msg1->isEnhancedChecksum, testMessage->isEnhancedChecksum);
	EXPECT_EQ(msg1->data, testMessage->data);
	EXPECT_EQ(msg1->checksum, testMessage->checksum);
	packets.pop_back();

	//LIN_01 frame from device
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage2 = std::dynamic_pointer_cast<icsneo::LINMessage>(decodeMsg);
	EXPECT_EQ(msg2->network, testMessage2->network);
	EXPECT_EQ(msg2->ID, testMessage2->ID);
	EXPECT_EQ(msg2->type, testMessage2->type);
	EXPECT_EQ(msg2->isEnhancedChecksum, testMessage2->isEnhancedChecksum);
	EXPECT_EQ(msg2->data, testMessage2->data);
	EXPECT_EQ(msg2->checksum, testMessage2->checksum);
}

static std::shared_ptr<LINMessage> decodeLinPacket(const HardwareLINPacket& pkt) {
	std::vector<uint8_t> bytes(sizeof(pkt));
	std::memcpy(bytes.data(), &pkt, sizeof(pkt));
	return std::dynamic_pointer_cast<LINMessage>(HardwareLINPacket::DecodeToMessage(bytes));
}

TEST_F(LINEncoderDecoderTest, WakeupPulseDecodeTest) {
	HardwareLINPacket pkt{};
	pkt.CoreMiniBitsLIN.WakeupRequest = 1;
	auto msg = decodeLinPacket(pkt);
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->linMsgType, LINMessage::Type::LIN_WAKEUP_REQUEST);
	EXPECT_TRUE(msg->statusFlags.WakeupRequest);
}

TEST_F(LINEncoderDecoderTest, WakeupFlagOnUartBreakStaysError) {
	HardwareLINPacket pkt{};
	pkt.CoreMiniBitsLIN.WakeupRequest = 1;
	pkt.CoreMiniBitsLIN.ErrRxOnlyBreak = 1;
	auto msg = decodeLinPacket(pkt);
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->linMsgType, LINMessage::Type::LIN_ERROR);
	EXPECT_TRUE(msg->statusFlags.WakeupRequest);
	EXPECT_TRUE(msg->errFlags.ErrRxBreakOnly);
}

TEST_F(LINEncoderDecoderTest, WakeupFlagOnCompleteFrameKeepsCommander) {
	HardwareLINPacket pkt{};
	pkt.CoreMiniBitsLIN.WakeupRequest = 1;
	pkt.CoreMiniBitsLIN.TXCommander = 1;
	pkt.CoreMiniBitsLIN.ID = 0x10;
	pkt.CoreMiniBitsLIN.len = 3; // two responder bytes plus the checksum
	pkt.data[0] = 0xaa;
	pkt.data[1] = 0xbb;
	pkt.data[2] = 0x99; // classic checksum, otherwise the frame decodes as LIN_ERROR
	auto msg = decodeLinPacket(pkt);
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->linMsgType, LINMessage::Type::LIN_COMMANDER_MSG);
	EXPECT_TRUE(msg->statusFlags.WakeupRequest);
	EXPECT_EQ(msg->ID, 0x10);
}
