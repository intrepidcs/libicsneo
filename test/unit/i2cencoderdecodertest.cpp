#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/i2cpacket.h"
#include "icsneo/communication/message/i2cmessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/core/ringbuffer.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>

using namespace icsneo;

class I2CEncoderDecoderTest : public ::testing::Test {
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

	//Read request to the device
	//Control length 1, control bytes 0x12 (I2C_01 register to read from)
	//data length 1: blank bytes padded in that the device will fill in the reply
	std::vector<uint8_t> testBytes =
	{0xaa, 0x0c, 0x11, 0x00, 0x58, 0x00, 0x01, 0x00,
	 0x01, 0x00, 0x00, 0x01, 0x68, 0x10, 0x12, 0x00};

	std::vector<uint8_t> recvBytes = 
	{0xaa, 0x0c, 0x24,0x00, 0x58, 0x00, 0x68, 0x18,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x97, 0x29,
	0xe6, 0xfb, 0xc1, 0xfc, 0xb0, 0x80, 0x35, 0x00,
	0x02, 0x00, 0x12, 0x80};
};

TEST_F(I2CEncoderDecoderTest, PacketEncoderTest) {
	std::vector<uint8_t> bytestream;
	auto message = std::make_shared<icsneo::I2CMessage>();
	message->network = icsneo::Network::NetID::I2C_01;
	message->controlBytes.push_back(static_cast<uint8_t>(0x12u)); //Product ID register address
	message->dataBytes.push_back(static_cast<uint8_t>(0x00u));
	message->address = 0x68u; //7 bit addressing, BASE_ADDR
	message->stats = static_cast<uint16_t>(0x0001u);
	message->direction = I2CMessage::Direction::Read;
	message->isTXMsg = true;
	packetEncoder->encode(*packetizer, bytestream, message);
	EXPECT_EQ(bytestream, testBytes);
}

TEST_F(I2CEncoderDecoderTest, PacketDecoderTest) {
	std::shared_ptr<icsneo::Message> decodeMsg;
	std::shared_ptr<icsneo::I2CMessage> message = std::make_shared<icsneo::I2CMessage>();

	message->network = icsneo::Network::NetID::I2C_01;
	message->controlBytes.push_back(static_cast<uint8_t>(0x12u)); //Product ID register address
	message->dataBytes.push_back(static_cast<uint8_t>(0x80u));
	message->address = 0x68u; //7 bit addressing, BASE_ADDR
	message->stats = static_cast<uint16_t>(0x0002u);
	message->direction = I2CMessage::Direction::Read;
	message->deviceMode = I2CMessage::DeviceMode::Controller;
	message->isTXMsg = true;
	message->timestamp = static_cast<uint64_t>(0xB0FCC1FBE62997);

	ringBuffer.clear();
	ringBuffer.write(recvBytes);
	EXPECT_TRUE(packetizer->input(ringBuffer));
	auto packets = packetizer->output();
	if(packets.empty()) { EXPECT_TRUE(false); }
	EXPECT_TRUE(packetDecoder->decode(decodeMsg, packets.back()));
	auto testMessage = std::dynamic_pointer_cast<icsneo::I2CMessage>(decodeMsg);
	EXPECT_EQ(message->network, testMessage->network);
	EXPECT_EQ(message->controlBytes, testMessage->controlBytes);
	EXPECT_EQ(message->dataBytes, testMessage->dataBytes);
	EXPECT_EQ(message->address, testMessage->address);
	EXPECT_EQ(message->stats, testMessage->stats);
	EXPECT_EQ(message->direction, testMessage->direction);
	EXPECT_EQ(message->deviceMode, testMessage->deviceMode);
	EXPECT_EQ(message->isTXMsg, testMessage->isTXMsg);
	EXPECT_EQ(message->timestamp, testMessage->timestamp);
}
