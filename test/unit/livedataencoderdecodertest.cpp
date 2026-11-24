#include "icsneo/icsneocpp.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/packet/livedatapacket.h"
#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/core/ringbuffer.h"
#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"
#include <vector>
#include <iostream>

using namespace icsneo;

class LiveDataEncoderDecoderTest : public ::testing::Test {
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

		msg = std::make_shared<icsneo::LiveDataCommandMessage>();
		arg = std::make_shared<LiveDataArgument>();
		msg->handle = 1;
		msg->updatePeriod = std::chrono::milliseconds(100);
		msg->expirationTime = std::chrono::milliseconds(0);
		arg->objectType = LiveDataObjectType::MISC;
		arg->objectIndex = 0;
		arg->signalIndex = 0;
		arg->valueType = LiveDataValueType::GPS_LATITUDE;
		msg->args.push_back(arg);
	}
	device_eventhandler_t report;
	std::optional<Encoder> packetEncoder;
	std::optional<Packetizer> packetizer;
	std::optional<Decoder> packetDecoder;
	RingBuffer ringBuffer = RingBuffer(128);

	const std::vector<uint8_t> testBytesSub =
	{
		0xaa, //start AA
		0x0B, //netid main51 
		0x30, 0x00, //size little end 16
		0xF0, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x26, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //live data version
		0x01, 0x00, 0x00, 0x00, //live data command (subscribe)
		0x01, 0x00, 0x00, 0x00, //live data handle
		0x01, 0x00, 0x00, 0x00, //numArgs
		0x64, 0x00, 0x00, 0x00, //freqMs (100ms)
		0x00, 0x00, 0x00, 0x00, //expireMs (zero, never expire)
		0x08, 0x00, //lObjectType eCoreMiniObjectTypeMisc
		0x00, 0x00, 0x00, 0x00, //lObjectIndex
		0x00, 0x00, 0x00, 0x00, //lSignalIndex
		0x02, 0x00, 0x00, 0x00, //enumCoreMiniMiscGPSLatitude
		0x41 //padding byte
	};

	const std::vector<uint8_t> testBytesUnsub =
	{
		0xaa, //start AA
		0x0B, //netid main51 
		0x16, 0x00, //size little end 16
		0xF0, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x0C, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //LiveDataUtil::LiveDataVersion
		0x02, 0x00, 0x00, 0x00, //LiveDataCommand::UNSUBSCRIBE
		0x01, 0x00, 0x00, 0x00, //handle
		0x41 //padding byte
	};

	const std::vector<uint8_t> testBytesClear =
	{
		0xaa, //start AA
		0x0B, //netid main51 
		0x16, 0x00, //size little end 16
		0xF0, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x0C, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //LiveDataUtil::LiveDataVersion
		0x04, 0x00, 0x00, 0x00, //LiveDataCommand::CLEAR_ALL
		0x00, 0x00, 0x00, 0x00, //handle
		0x41 //padding byte
	};

	const std::vector<uint8_t> testBytesResponse =
	{
		0xaa, //start AA
		0x0C, //netid RED
		0x2C, 0x00, //size little end 16
		0xF0, 0x00, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x1C, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //version
		0x03, 0x00, 0x00, 0x00, //cmd
		0x01, 0x00, 0x00, 0x00, //handle
		0x01, 0x00, 0x00, 0x00, //numArgs
		0x08, 0x00, //value 1 header (length)
		0x00, //value 1 format
		0x00, //value 1 reserved
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //value large
		0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	const std::vector<uint8_t> testBytesStatus =
	{
		0xaa, //start AA
		0x0C, //netid RED
		0x24, 0x00, //size little end 16
		0xF0, 0x00, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x14, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //version
		0x00, 0x00, 0x00, 0x00, //cmd (status)
		0x01, 0x00, 0x00, 0x00, //handle
		0x01, 0x00, 0x00, 0x00, //requested command (subscribe)
		0x00, 0x00, 0x00, 0x00, //error
		0x00, 0x00, 0x00, 0x00, //padding
		0x00, 0x00,
	};

	const std::vector<uint8_t> testBytesSetValue =
	{
		0xaa, //start AA
		0x0B, //netid main51
		0x4E, 0x00, //size little end 16
		0xF0, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x44, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //LiveDataUtil::LiveDataVersion
		0x05, 0x00, 0x00, 0x00, //LiveDataCommand::SET_VALUE
		0x01, 0x00, 0x00, 0x00, //handle
		0x02, 0x00, 0x00, 0x00, //numSetValues
		0x08, 0x00, //lObjectType eCoreMiniObjectTypeMisc
		0x00, 0x00, 0x00, 0x00, //lObjectIndex
		0x00, 0x00, 0x00, 0x00, //lSignalIndex
		0x6C, 0x00, 0x00, 0x00, //enumCoreMiniMiscManualTrigger
		0x08, 0x00, //value 1 header (length)
		0x00, //value 1 format (FIXED_32_32)
		0x00, //value 1 reserved
		0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, //value large (7.0)
		0x08, 0x00, //lObjectType eCoreMiniObjectTypeMisc
		0x00, 0x00, 0x00, 0x00, //lObjectIndex
		0x00, 0x00, 0x00, 0x00, //lSignalIndex
		0x85, 0x00, 0x00, 0x00, //enumCoreMiniMiscCmCreateTime
		0x08, 0x00, //value 2 header (length)
		0x01, //value 2 format (UNSIGNED_64)
		0x00, //value 2 reserved
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, //value large (0x8000000000000001)
		0x41 //padding byte
	};

	const std::vector<uint8_t> testBytesResponseFormats =
	{
		0xaa, //start AA
		0x0C, //netid RED
		0x38, 0x00, //size little end 16
		0xF0, 0x00, //extended header command
		0x35, 0x00, //Live data subcommand little 16
		0x28, 0x00, //extended subcommand size, little 16
		0x01, 0x00, 0x00, 0x00, //version
		0x03, 0x00, 0x00, 0x00, //cmd
		0x01, 0x00, 0x00, 0x00, //handle
		0x02, 0x00, 0x00, 0x00, //numArgs
		0x08, 0x00, //value 1 header (length)
		0x01, //value 1 format (UNSIGNED_64)
		0x00, //value 1 reserved
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, //value large (0x8000000000000001)
		0x08, 0x00, //value 2 header (length)
		0x00, //value 2 format (FIXED_32_32)
		0x00, //value 2 reserved
		0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, //value large (1.5)
		0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	std::shared_ptr<icsneo::LiveDataCommandMessage> msg;
	std::shared_ptr<icsneo::LiveDataArgument> arg;
};

TEST_F(LiveDataEncoderDecoderTest, EncodeSubscribeCommandTest) {
	std::vector<uint8_t> bytestream;
	msg->cmd = icsneo::LiveDataCommand::SUBSCRIBE;
	packetEncoder->encode(*packetizer, bytestream, msg);
	EXPECT_EQ(bytestream, testBytesSub);
}

TEST_F(LiveDataEncoderDecoderTest, EncodeUnsubscribeCommandTest) {
	std::vector<uint8_t> bytestream;
	auto unsubMsg = std::make_shared<icsneo::LiveDataMessage>();
	unsubMsg->cmd = icsneo::LiveDataCommand::UNSUBSCRIBE;
	unsubMsg->handle = msg->handle;
	packetEncoder->encode(*packetizer, bytestream, unsubMsg);
	EXPECT_EQ(bytestream, testBytesUnsub);
}

TEST_F(LiveDataEncoderDecoderTest, EncodeClearCommandTest) {
	std::vector<uint8_t> bytestream;
	auto unsubMsg = std::make_shared<icsneo::LiveDataMessage>();
	unsubMsg->cmd = icsneo::LiveDataCommand::CLEAR_ALL;
	packetEncoder->encode(*packetizer, bytestream, unsubMsg);
	EXPECT_EQ(bytestream, testBytesClear);
}

TEST_F(LiveDataEncoderDecoderTest, DecoderStatusTest) {
	std::shared_ptr<Message> result;
	ringBuffer.clear();
	ringBuffer.write(testBytesStatus);
	if (packetizer->input(ringBuffer)) {
		for (const auto& packet : packetizer->output()) {
			if (!packetDecoder->decode(result, packet))
				continue;
		}
	}
	EXPECT_TRUE(result != nullptr);
	auto response = std::dynamic_pointer_cast<LiveDataStatusMessage>(result);
	EXPECT_EQ(response->handle, static_cast<uint32_t>(1u));
	EXPECT_EQ(response->cmd, LiveDataCommand::STATUS);
	EXPECT_EQ(response->requestedCommand, LiveDataCommand::SUBSCRIBE);
	EXPECT_EQ(response->status, LiveDataStatus::SUCCESS);
}

TEST_F(LiveDataEncoderDecoderTest, DecoderResponseTest) {
	std::shared_ptr<Message> result;
	ringBuffer.clear();
	ringBuffer.write(testBytesResponse);
	if (packetizer->input(ringBuffer)) {
		for (const auto& packet : packetizer->output()) {
			if (!packetDecoder->decode(result, packet))
				continue;
		}
	}
	EXPECT_TRUE(result != nullptr);
	auto response = std::dynamic_pointer_cast<LiveDataValueMessage>(result);
	EXPECT_EQ(response->handle, static_cast<uint32_t>(1u));
	EXPECT_EQ(response->cmd, LiveDataCommand::RESPONSE);
	EXPECT_EQ(response->numArgs, static_cast<uint32_t>(1u));
	EXPECT_EQ(icsneo::LiveDataUtil::liveDataValueToDouble(*response->values[0]), 0.0);
}

TEST_F(LiveDataEncoderDecoderTest, EncodeSetValueCommandTest) {
	std::vector<uint8_t> bytestream;
	auto setValMsg = std::make_shared<icsneo::LiveDataSetValueMessage>();
	setValMsg->cmd = icsneo::LiveDataCommand::SET_VALUE;
	setValMsg->handle = msg->handle;
	auto fixed = icsneo::LiveDataUtil::liveDataDoubleToValue(7.0);
	ASSERT_TRUE(fixed.has_value());
	auto integer = icsneo::LiveDataUtil::liveDataUInt64ToValue(0x8000000000000001ull);
	setValMsg->appendSetValue(icsneo::LiveDataValueType::MANUAL_TRIGGER, *fixed);
	setValMsg->appendSetValue(icsneo::LiveDataValueType::LOGGER_CREATE_TIME, integer);
	packetEncoder->encode(*packetizer, bytestream, setValMsg);
	EXPECT_EQ(bytestream, testBytesSetValue);
}

TEST_F(LiveDataEncoderDecoderTest, DecoderResponseFormatsTest) {
	std::shared_ptr<Message> result;
	ringBuffer.clear();
	ringBuffer.write(testBytesResponseFormats);
	if (packetizer->input(ringBuffer)) {
		for (const auto& packet : packetizer->output()) {
			if (!packetDecoder->decode(result, packet))
				continue;
		}
	}
	EXPECT_TRUE(result != nullptr);
	auto response = std::dynamic_pointer_cast<LiveDataValueMessage>(result);
	EXPECT_EQ(response->handle, static_cast<uint32_t>(1u));
	EXPECT_EQ(response->cmd, LiveDataCommand::RESPONSE);
	EXPECT_EQ(response->numArgs, static_cast<uint32_t>(2u));
	ASSERT_EQ(response->values.size(), static_cast<size_t>(2u));
	EXPECT_EQ(response->values[0]->header.format, LiveDataValueFormat::UNSIGNED_64);
	EXPECT_EQ(icsneo::LiveDataUtil::liveDataValueToUInt64(*response->values[0]), 0x8000000000000001ull);
	EXPECT_FALSE(icsneo::LiveDataUtil::liveDataValueToDouble(*response->values[0]).has_value());
	EXPECT_EQ(response->values[1]->header.format, LiveDataValueFormat::FIXED_32_32);
	EXPECT_EQ(icsneo::LiveDataUtil::liveDataValueToDouble(*response->values[1]), 1.5);
	EXPECT_FALSE(icsneo::LiveDataUtil::liveDataValueToUInt64(*response->values[1]).has_value());
}

TEST(LiveDataUtilTest, ConversionsMatchTheValueFormat) {
	auto fixed = LiveDataUtil::liveDataDoubleToValue(1.5);
	ASSERT_TRUE(fixed.has_value());
	EXPECT_EQ(fixed->header.format, LiveDataValueFormat::FIXED_32_32);
	EXPECT_EQ(LiveDataUtil::liveDataValueToDouble(*fixed), 1.5);
	EXPECT_FALSE(LiveDataUtil::liveDataValueToUInt64(*fixed).has_value());

	// above 2^63, so read signed it would be negative, and above 2^53, so a double would round it
	const uint64_t big = 0x8000000000000001ull;
	auto integer = LiveDataUtil::liveDataUInt64ToValue(big);
	EXPECT_EQ(integer.header.format, LiveDataValueFormat::UNSIGNED_64);
	EXPECT_EQ(integer.header.length, sizeof(integer.value));
	EXPECT_EQ(LiveDataUtil::liveDataValueToUInt64(integer), big);
	EXPECT_FALSE(LiveDataUtil::liveDataValueToDouble(integer).has_value());
}
