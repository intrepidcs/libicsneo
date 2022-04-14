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

TEST_F(DiskDriverTest, ReadZero) {
	uint8_t b = 0xCDu;
	const auto amountRead = readLogicalDisk(0, &b, 0);
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, 0u);
	EXPECT_EQ(b, 0xCDu);
	EXPECT_EQ(driver->readCalls, 0u);
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

TEST_F(DiskDriverTest, ReadCache) {
	std::array<uint8_t, 128> buf;
	buf.fill(0u);
	auto amountRead = readLogicalDisk(1, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], TEST_STRING[1]);
	EXPECT_EQ(buf[110], 111u);
	EXPECT_EQ(driver->readCalls, 1u);

	// Subsequent reads (within the same second) should hit the cache
	amountRead = readLogicalDisk(1, buf.data(), buf.size());
	EXPECT_EQ(driver->readCalls, 1u);

	// The underlying data can be changed
	driver->mockDisk[1] = 'J';

	// But the same data should be returned from the cache
	amountRead = readLogicalDisk(1, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], TEST_STRING[1]);
	EXPECT_EQ(buf[110], 111u);
	EXPECT_EQ(driver->readCalls, 1u);

	driver->invalidateCache(0, 0xfffff);

	// After invalidating the cache (or waiting for it to expire), the underlying data will be read
	amountRead = readLogicalDisk(1, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], 'J');
	EXPECT_EQ(buf[110], 111u);
	EXPECT_EQ(driver->readCalls, 2u);
}

TEST_F(DiskDriverTest, ReadCacheLong) {
	std::array<uint8_t, 500> buf;
	buf.fill(0u);
	auto amountRead = readLogicalDisk(300, buf.data(), buf.size());
	EXPECT_TRUE(amountRead.has_value());
	EXPECT_EQ(amountRead, buf.size());
	EXPECT_EQ(buf[0], 300 & 0xFF);
	EXPECT_EQ(buf[110], 410 & 0xFF);
	EXPECT_EQ(driver->readCalls, 3u);

	// Re-read the end, it will be in the cache
	amountRead = readLogicalDisk(780, buf.data() + 480, buf.size() - 480);
	EXPECT_EQ(buf[490], 790 & 0xFF);
	EXPECT_EQ(driver->readCalls, 3u);
}