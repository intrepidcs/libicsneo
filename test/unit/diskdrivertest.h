#ifndef __DISKDRIVERTEST_H_
#define __DISKDRIVERTEST_H_

#include "icsneo/disk/diskreaddriver.h"
#include "icsneo/disk/diskwritedriver.h"
#include "gtest/gtest.h"
#include <queue>
#include <functional>
#include <optional>

using namespace icsneo;

#define TEST_STRING "The quick brown fox jumps over the lazy dog."
#define TEST_OVERWRITE_STRING "test fun"

class MockDiskDriver : public Disk::ReadDriver, public Disk::WriteDriver {
public:
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override { return { 8, 256 }; }

	std::optional<uint64_t> readLogicalDiskAligned(Communication&, device_eventhandler_t,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds, Disk::MemoryType) override {
		readCalls++;

		EXPECT_EQ(pos % getBlockSizeBounds().first, 0); // Ensure the alignment rules are respected
		EXPECT_LE(amount, getBlockSizeBounds().second);
		EXPECT_EQ(amount % getBlockSizeBounds().first, 0);

		if(pos > mockDisk.size()) // EOF
			return std::nullopt;

		std::optional<uint64_t> readAmount = std::min(amount, mockDisk.size() - pos);
		if(readAmount > 0u)
			memcpy(into, mockDisk.data() + pos, static_cast<size_t>(*readAmount));

		// So that the test can mess with atomicity
		if(afterReadHook)
			afterReadHook();

		return readAmount;
	}

	std::optional<uint64_t> writeLogicalDiskAligned(Communication&, device_eventhandler_t report, uint64_t pos,
		const uint8_t* from, uint64_t amount, std::chrono::milliseconds, Disk::MemoryType) override {
		writeCalls++;

		EXPECT_EQ(pos % getBlockSizeBounds().first, 0); // Ensure the alignment rules are respected
		EXPECT_LE(amount, getBlockSizeBounds().second);
		EXPECT_EQ(amount % getBlockSizeBounds().first, 0);

		if(pos > mockDisk.size()) // EOF
			return std::nullopt;

		std::optional<uint64_t> writeAmount = std::min(amount, mockDisk.size() - pos);
		if(writeAmount > 0u) {
			memcpy(mockDisk.data() + pos, from, static_cast<size_t>(*writeAmount));
		}
		return writeAmount;
	}

	std::array<uint8_t, 1024> mockDisk;
	size_t readCalls = 0;
	size_t writeCalls = 0;
	std::function<void(void)> afterReadHook;

private:
	Disk::Access getPossibleAccess() const override { return Disk::Access::EntireCard; }
};

class DiskDriverTest : public ::testing::Test {
protected:
	// Start with a clean instance of MockDiskDriver for every test
	void SetUp() override {
		onError = [this](APIEvent::Type t, APIEvent::Severity s) {
			if(expectedErrors.empty()) {
				// Unless caught by the test, the driver should not throw errors
				EXPECT_TRUE(false);
			} else {
				const auto expected = expectedErrors.front();
				expectedErrors.pop();
				EXPECT_EQ(expected.first, t);
				if(expected.second != APIEvent::Severity::Any) {
					EXPECT_EQ(expected.second, s);
				}
			}
		};
		driver.emplace();

		// Populate with some fake data
		memcpy(driver->mockDisk.data(), TEST_STRING, sizeof(TEST_STRING));
		for (size_t i = sizeof(TEST_STRING); i < driver->mockDisk.size(); i++)
			driver->mockDisk[i] = uint8_t(i & 0xFF);
	}

	void TearDown() override {
		driver.reset();
	}

	std::optional<uint64_t> readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount) {
		return driver->readLogicalDisk(*com, onError, pos, into, amount /* default timeout */);
	}

	std::optional<uint64_t> writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount) {
		return driver->writeLogicalDisk(*com, onError, *driver, pos, from, amount /* default timeout */);
	}

	std::optional<MockDiskDriver> driver;

	std::queue< std::pair<APIEvent::Type, APIEvent::Severity> > expectedErrors;
	device_eventhandler_t onError;

	// We will dereference this but the driver base should never access it
	Communication* const com = nullptr;
};

#endif // __DISKDRIVERTEST_H_