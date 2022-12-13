#include <array>
#include <thread>
#include "gtest/gtest.h"
#include "icsneo/communication/socket.h"

using namespace icsneo;

static constexpr uint16_t TEST_PORT = 55555;

TEST(DeviceSharingSocketTest, SocketOpen) {
	ActiveSocket socket(SocketBase::Protocol::TCP, TEST_PORT);
	EXPECT_TRUE(socket.open());
	EXPECT_TRUE(socket.close());
}

TEST(DeviceSharingSocketTest, SocketOpenAndConnect) {
	Acceptor acceptor(SocketBase::Protocol::TCP, TEST_PORT);
	ActiveSocket socket(SocketBase::Protocol::TCP, TEST_PORT);
	acceptor.initialize();

	EXPECT_TRUE(socket.open());
	EXPECT_TRUE(socket.connect());
	EXPECT_TRUE(socket.close());
}

TEST(DeviceSharingSocketTest, SocketReadWrite) {
	std::array<uint8_t, 4> testData = {0xDE, 0xAD, 0xBE, 0xEF};
	std::array<uint8_t, 4> readData;
	Acceptor acceptor(SocketBase::Protocol::TCP, TEST_PORT);
	acceptor.initialize();

	ActiveSocket socket(SocketBase::Protocol::TCP, TEST_PORT);
	EXPECT_TRUE(socket.open());
	EXPECT_TRUE(socket.connect());

	auto acceptSocket = acceptor.accept();

	EXPECT_TRUE(socket.write(testData.data(), testData.size()));
	EXPECT_TRUE(acceptSocket->read(readData.data(), readData.size()));
	EXPECT_EQ(testData, readData);
	EXPECT_TRUE(socket.close());
	EXPECT_TRUE(acceptSocket->close());
}
