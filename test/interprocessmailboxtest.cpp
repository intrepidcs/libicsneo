#include <thread>

#include "gtest/gtest.h"
#include "icsneo/communication/interprocessmailbox.h"
#include "icsneo/icsneocpp.h"

using namespace icsneo;

static std::chrono::milliseconds TIMEOUT(10'000);

// ensures that the shared-memory and shared-semaphores get cleared
TEST(InterprocessMailboxTest, CreateDestroy) {
	static constexpr auto name = "icsneo-test";
	{
		InterprocessMailbox mb;
		EXPECT_TRUE(mb.open(name, true));
		EXPECT_TRUE(mb.close());
	}
	{
		InterprocessMailbox mb;
		EXPECT_FALSE(mb.open(name)); // create == false
		auto err = icsneo::GetLastError();
		EXPECT_EQ(err.getType(), APIEvent::Type::SharedSemaphoreFailedToOpen);
		EXPECT_EQ(err.getSeverity(), APIEvent::Severity::Error);
		EXPECT_FALSE(mb.close()); // never opened successfully
	}
}

// these test should really be done in separate processes
TEST(InterprocessMailboxTest, Looping) {
	size_t data = SIZE_MAX; // just dummy data
	InterprocessMailbox in;
	InterprocessMailbox out;
	EXPECT_TRUE(in.open("icsneo-test", true));
	EXPECT_TRUE(out.open("icsneo-test"));
	for(unsigned i = 0; i < MESSAGE_COUNT * 64; ++i) {
		// "send" the message
		EXPECT_TRUE(out.write(&data, sizeof(data), TIMEOUT));
		uint8_t buff[MAX_DATA_SIZE];
		LengthFieldType read;
		EXPECT_TRUE(in.read(buff, read, TIMEOUT));
		EXPECT_EQ(*(decltype(data)*)buff, data);
		--data;
	}
	EXPECT_TRUE(in.close());
	EXPECT_TRUE(out.close());
}

TEST(InterprocessMailboxTest, FasterSender) {
	std::thread sender([] {
		InterprocessMailbox out;
		EXPECT_TRUE(out.open("icsneo-test", true));

		// nested here to spawn after the sender is set up
		std::thread receiver([] {
			InterprocessMailbox in;
			EXPECT_TRUE(in.open("icsneo-test"));

			// wait for the sender to fill up
			std::this_thread::sleep_for(std::chrono::seconds(2));
			uint8_t buff[MAX_DATA_SIZE];
			LengthFieldType read;

			// the messages should be 1, 2, 3, ..., MESSAGE_COUNT, this proves the sender waited
			for(unsigned i = 0; i < MESSAGE_COUNT; ++i) {
				EXPECT_TRUE(in.read(buff, read, TIMEOUT));
				EXPECT_EQ(*(decltype(i)*)buff, i);
			}

			// make sure we can do it again for the rest
			for(unsigned i = MESSAGE_COUNT; i < MESSAGE_COUNT * 2; ++i) {
				EXPECT_TRUE(in.read(buff, read, TIMEOUT));
				EXPECT_EQ(*(decltype(i)*)buff, i);
			}
			EXPECT_TRUE(in.close());
		});

		// try to send two times the count, we'll end up blocked at the end till the receiver unblocks
		for(unsigned i = 0; i < MESSAGE_COUNT * 2; ++i) {
			EXPECT_TRUE(out.write(&i, sizeof(i), TIMEOUT));
		}

		receiver.join();
		EXPECT_TRUE(out.close());
	});
	sender.join();
}
