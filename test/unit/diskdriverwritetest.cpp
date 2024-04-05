#include "diskdrivertest.h"

TEST_F(DiskDriverTest, Write) {
	const auto amountWritten = writeLogicalDisk(0u, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, sizeof(TEST_OVERWRITE_STRING));
	EXPECT_STREQ(reinterpret_cast<char*>(driver->mockDisk.data()), TEST_OVERWRITE_STRING);
	EXPECT_EQ(driver->mockDisk[sizeof(TEST_OVERWRITE_STRING) + 1], TEST_STRING[sizeof(TEST_OVERWRITE_STRING) + 1]);
	EXPECT_EQ(driver->mockDisk[126], 126u);
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 1u);
}

TEST_F(DiskDriverTest, WriteZero) {
	uint8_t b = 0xCDu;
	const auto amountWritten = writeLogicalDisk(0, &b, 0);
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, 0u);
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->readCalls, 0u);
	EXPECT_EQ(driver->writeCalls, 0u);
}

TEST_F(DiskDriverTest, WriteUnaligned) {
	const auto amountWritten = writeLogicalDisk(3, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, sizeof(TEST_OVERWRITE_STRING));
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->mockDisk[5], TEST_OVERWRITE_STRING[2]);
	EXPECT_EQ(driver->mockDisk[110], 110u);
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 1u);
}

TEST_F(DiskDriverTest, WriteUnalignedLong) {
	std::array<uint8_t, 500> buf;
	for(size_t i = 0; i < buf.size(); i++)
		buf[i] = static_cast<uint8_t>((buf.size() - i) + 20);
	const auto amountWritten = writeLogicalDisk(300, buf.data(), buf.size());
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, buf.size());
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->mockDisk[330], ((buf.size() - 30) + 20) & 0xFF);
	EXPECT_EQ(driver->readCalls, 2u);
	EXPECT_EQ(driver->writeCalls, 3u);
}

TEST_F(DiskDriverTest, WritePastEnd) {
	expectedErrors.push({ APIEvent::Type::EOFReached, APIEvent::Severity::Error });
	const auto amountWritten = writeLogicalDisk(1020, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, 4u);
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->mockDisk[1019], 1019 & 0xFF);
	EXPECT_EQ(driver->mockDisk[1020], TEST_OVERWRITE_STRING[0]);
	EXPECT_EQ(driver->mockDisk[1023], TEST_OVERWRITE_STRING[3]);
	EXPECT_EQ(driver->writeCalls, 1u); // One for the write, another to check EOF
}

TEST_F(DiskDriverTest, WriteBadStartingPos) {
	expectedErrors.push({ APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error });
	const auto amountWritten = writeLogicalDisk(2000, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_FALSE(amountWritten.has_value());
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 0u); // We never even attempt the write
}