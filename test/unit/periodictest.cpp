#include "icsneo/api/periodic.h"
#include "gtest/gtest.h"

#include <condition_variable>

using namespace icsneo;

// no wait, make sure stop works
TEST(PeriodicTest, StartStop)
{
	const auto start = std::chrono::steady_clock::now();
	Periodic p([] { return true; }, std::chrono::milliseconds(1000));
	const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
	EXPECT_LT(delta.count(), 100); // hopefully enough
}

// time single cycle
TEST(PeriodicTest, OneCycle)
{
	std::condition_variable cv;
	std::mutex mutex;
	uint8_t cycles = 0;
	const auto start = std::chrono::steady_clock::now();
	{
		Periodic p([&] {
			{
				std::scoped_lock lk(mutex);
				++cycles;
			}
			cv.notify_one();
			return true;
		}, std::chrono::seconds(1));
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait_for(lk, std::chrono::seconds(2), [&]{ return cycles > 0; });
	}
	const auto delta = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
	EXPECT_EQ(delta.count(), 1);
	EXPECT_EQ(cycles, 1);
}

TEST(PeriodicTest, TenCycles)
{
	std::condition_variable cv;
	std::mutex mutex;
	uint8_t cycles = 0;
	const auto start = std::chrono::steady_clock::now();
	{
		Periodic p([&] {
			{
				std::scoped_lock lk(mutex);
				++cycles;
			}
			cv.notify_one();
			return true;
		}, std::chrono::milliseconds(100));
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait_for(lk, std::chrono::seconds(2), [&]{ return cycles >= 10; });
	}
	const auto delta = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
	EXPECT_EQ(delta.count(), 1);
	EXPECT_EQ(cycles, 10);
}
