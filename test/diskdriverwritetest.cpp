#include "diskdrivertest.h"

TEST_F(DiskDriverTest, Write) {
	const auto amountWritten = writeLogicalDisk(0u, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, sizeof(TEST_OVERWRITE_STRING));
	EXPECT_STREQ(reinterpret_cast<char*>(driver->mockDisk.data()), TEST_OVERWRITE_STRING);
	EXPECT_EQ(driver->mockDisk[sizeof(TEST_OVERWRITE_STRING) + 1], TEST_STRING[sizeof(TEST_OVERWRITE_STRING) + 1]);
	EXPECT_EQ(driver->mockDisk[126], 126u);
	EXPECT_EQ(driver->atomicityChecks, 1u);
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 1u);
}

TEST_F(DiskDriverTest, WriteNoAtomicityCheck) {
	driver->supportsAtomic = false;
	expectedErrors.push({ APIEvent::Type::AtomicOperationCompletedNonatomically, APIEvent::Severity::EventInfo });
	const auto amountWritten = writeLogicalDisk(0u, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, sizeof(TEST_OVERWRITE_STRING));
	EXPECT_STREQ(reinterpret_cast<char*>(driver->mockDisk.data()), TEST_OVERWRITE_STRING);
	EXPECT_EQ(driver->mockDisk[sizeof(TEST_OVERWRITE_STRING) + 1], TEST_STRING[sizeof(TEST_OVERWRITE_STRING) + 1]);
	EXPECT_EQ(driver->mockDisk[126], 126u);
	EXPECT_EQ(driver->atomicityChecks, 0u);
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 1u);
}

TEST_F(DiskDriverTest, WriteUnaligned) {
	const auto amountWritten = writeLogicalDisk(3, reinterpret_cast<const uint8_t*>(TEST_OVERWRITE_STRING), sizeof(TEST_OVERWRITE_STRING));
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, sizeof(TEST_OVERWRITE_STRING));
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->mockDisk[5], TEST_OVERWRITE_STRING[2]);
	EXPECT_EQ(driver->mockDisk[110], 110u);
	EXPECT_EQ(driver->atomicityChecks, 1u);
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
	EXPECT_EQ(driver->atomicityChecks, 3u);
	EXPECT_EQ(driver->readCalls, 3u);
	EXPECT_EQ(driver->writeCalls, 3u);
}

TEST_F(DiskDriverTest, WriteUnalignedLongAtomicityFailures) {
	std::array<uint8_t, 500> buf;
	for(size_t i = 0; i < buf.size(); i++)
		buf[i] = static_cast<uint8_t>((buf.size() - i) + 20);
	for(int i = 0; i < 4; i++)
		expectedErrors.push({ APIEvent::Type::AtomicOperationRetried, APIEvent::Severity::EventInfo });

	int i = 0;
	driver->afterReadHook = [&i, this]() {
		switch(i) {
		case 0: driver->mockDisk[295] = uint8_t(0xCD); break;
		case 1: break; // We don't mess with this one so the first block can be written
		case 2: driver->mockDisk[600] = uint8_t(0xDC); break;
		case 3: driver->mockDisk[602] = uint8_t(0xDC); break;
		case 4: break; // We don't mess with this one so the second block can be written
		case 5: driver->mockDisk[777] = uint8_t(0x22); break;
		case 6: break; // We don't mess with this one so the third block can be written
		}
		i++;
	};

	const auto amountWritten = writeLogicalDisk(300, buf.data(), buf.size());
	EXPECT_TRUE(amountWritten.has_value());
	EXPECT_EQ(amountWritten, buf.size());
	EXPECT_EQ(driver->mockDisk[0], TEST_STRING[0]);
	EXPECT_EQ(driver->mockDisk[295], 0xCDu); // If the atomic worked correctly this write won't have gotten trampled
	// Our writes happen after both of these, so they overwrite the 0xDC values
	EXPECT_EQ(driver->mockDisk[600], ((buf.size() - 300) + 20) & 0xFF);
	EXPECT_EQ(driver->mockDisk[602], ((buf.size() - 302) + 20) & 0xFF);

	EXPECT_EQ(driver->atomicityChecks, 7u);
	EXPECT_EQ(driver->readCalls, 7u);
	EXPECT_EQ(driver->writeCalls, 7u);
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
	EXPECT_EQ(driver->atomicityChecks, 0u);
	EXPECT_EQ(driver->readCalls, 1u);
	EXPECT_EQ(driver->writeCalls, 0u); // We never even attempt the write
}