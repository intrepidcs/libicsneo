#include "diskdrivertest.h"

TEST_F(DiskDriverTest, Read) {
	std::array<uint8_t, 128> buf;
	buf.fill(0u);
	const auto amountRead = readLogicalDisk(0, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], TEST_STRING[0]);
	EXPECT_EQ(buf[126], 126u);
	EXPECT_EQ(driver->readCalls, 1u);
}

TEST_F(DiskDriverTest, ReadUnaligned) {
	std::array<uint8_t, 120> buf;
	buf.fill(0u);
	const auto amountRead = readLogicalDisk(1, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], TEST_STRING[1]);
	EXPECT_EQ(buf[110], 111u);
	EXPECT_EQ(driver->readCalls, 1u);
}

TEST_F(DiskDriverTest, ReadUnalignedLong) {
	std::array<uint8_t, 500> buf;
	buf.fill(0u);
	const auto amountRead = readLogicalDisk(300, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], 300 & 0xFF);
	EXPECT_EQ(buf[110], 410 & 0xFF);
	EXPECT_EQ(driver->readCalls, 3u);
}

TEST_F(DiskDriverTest, ReadPastEnd) {
	std::array<uint8_t, 500> buf;
	buf.fill(0u);
	expectedErrors.push({ APIEvent::Type::EOFReached, APIEvent::Severity::Error });
	const auto amountRead = readLogicalDisk(1000, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, 24u);
	EXPECT_EQ(buf[0], 1000 & 0xFF);
	EXPECT_EQ(buf[23], 1023 & 0xFF);
	EXPECT_EQ(driver->readCalls, 2u); // One for the read, another to check EOF
}

TEST_F(DiskDriverTest, ReadBadStartingPos) {
	std::array<uint8_t, 500> buf;
	buf.fill(0u);
	expectedErrors.push({ APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error });
	const auto amountRead = readLogicalDisk(2000, buf.data(), buf.size());
	EXPECT_FALSE(amountRead.has_value());
	EXPECT_EQ(driver->readCalls, 1u); // One to check EOF
}