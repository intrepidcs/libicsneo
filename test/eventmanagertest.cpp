#include <thread>
#include <memory>

#include "icsneo/icsneocpp.h"
#include "gtest/gtest.h"

using namespace icsneo;

class EventManagerTest : public ::testing::Test {
protected:
    // Start with a clean instance of eventmanager for every test
    void SetUp() override {
        EventManager::ResetInstance();
    }
};


TEST_F(EventManagerTest, SingleThreadCallbacksTest) {
    int callCounter = 0;
    
	// increments counter when baudrate events show up
    int id1 = EventManager::GetInstance().addEventCallback(EventCallback([&callCounter](std::shared_ptr<APIEvent>){
        callCounter++;
    }, EventFilter(APIEvent::Type::BaudrateNotFound)));

	// increments counter when infos show up
	int id2 = EventManager::GetInstance().addEventCallback(EventCallback([&callCounter](std::shared_ptr<APIEvent>) {
		callCounter++;
	}, EventFilter(APIEvent::Severity::EventInfo)));

	EXPECT_EQ(callCounter, 0);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::EventWarning));

	EXPECT_EQ(callCounter, 0);

    EventManager::GetInstance().add(APIEvent(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventWarning));

	EXPECT_EQ(callCounter, 1);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 2);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 4);

	EXPECT_EQ(EventManager::GetInstance().removeEventCallback(id2), true);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 4);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 5);

    // increments counter when device currently open shows up
	int id3 = EventManager::GetInstance().addEventCallback(EventCallback([&callCounter](std::shared_ptr<APIEvent>) {
		callCounter++;
	}, EventFilter(APIEvent::Type::DeviceCurrentlyOpen)));

    EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 6);
    
    EXPECT_EQ(EventManager::GetInstance().removeEventCallback(id2), false);
	EXPECT_EQ(EventManager::GetInstance().removeEventCallback(id1), true);
	
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo));

    EXPECT_EQ(callCounter, 6);

	EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::EventInfo));

	EXPECT_EQ(callCounter, 7);

    EXPECT_EQ(EventManager::GetInstance().removeEventCallback(id3), true);

    EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::EventInfo));

    EXPECT_EQ(callCounter, 7);
}

TEST_F(EventManagerTest, ErrorDowngradingTest) {
	// Check that main thread has no errors
	EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

	// Adds 500 {OutputTruncated, Warning} and 500 {OutputTruncated, Info}
	// Adds and checks errors as well.
	std::thread t1([]() {
		for(int i = 0; i < 500; i++) {
			EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
			EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
		}

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

		EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);

		EventManager::GetInstance().downgradeErrorsOnCurrentThread();
		EventManager::GetInstance().add(APIEvent(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error));
		
		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);
		auto events = GetEvents(EventFilter(APIEvent::Type::BufferInsufficient, APIEvent::Severity::EventWarning));
		EXPECT_EQ(events.empty(), false);

		EventManager::GetInstance().cancelErrorDowngradingOnCurrentThread();

		EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
	});

	// Adds 500 {OutputTruncated, Warning} and 500 {OutputTruncated, Info}
	// Adds and checks errors as well.
	std::thread t2([]() {
		for(int i = 0; i < 500; i++) {
			EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
			EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
		}

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

		EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
		EventManager::GetInstance().add(APIEvent(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error));

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::BufferInsufficient);

		EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));

		EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
	});

	t1.join();
	t2.join();
}

/**
 * Adds a total of 3000 events from 3 different threads, checking that all were correctly added after all threads are joined.
 * Also adds errors from each of the 3 threads, checking that the last error is correct for that thread and that the main thread has no errors.
 */
TEST_F(EventManagerTest, MultithreadedTest) {

    // Check that main thread has no errors
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

    // Adds 500 {OutputTruncated, Warning} and 500 {OutputTruncated, Info}
    // Adds and checks errors as well.
    std::thread t1( []() {
        for(int i = 0; i < 500; i++) {
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
        }

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error));

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::BufferInsufficient);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
        
        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
    });

    // Check that main thread has no errors
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

    // Adds 500 {CANFDNotSupported, Warning} and 500 {CANFDSettingsNotAvailable, Info}
    // Adds and checks errors as well.
    std::thread t2( []() {
        for(int i = 0; i < 500; i++) {
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::CANFDNotSupported, APIEvent::Severity::EventWarning));
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::EventInfo));
        }

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error));
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error));

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::DeviceCurrentlyOffline);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::DeviceCurrentlyOnline, APIEvent::Severity::Error));
        
        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::DeviceCurrentlyOnline);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error));
    });

    // Check that main thread has no errors
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

    // Adds 500 {CANFDNotSupported, Warning} and 500 {FailedToWrite, Info}
    // Adds and checks errors as well.
    std::thread t3( []() {
        for(int i = 0; i < 500; i++) {
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::CANFDNotSupported, APIEvent::Severity::EventWarning));
            EventManager::GetInstance().add(APIEvent(APIEvent::Type::FailedToWrite, APIEvent::Severity::EventInfo));
        }

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::NoSerialNumber, APIEvent::Severity::Error));
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error));

        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::SettingsChecksumError);

        EventManager::GetInstance().add(APIEvent(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::Error));
        
        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::SWCANSettingsNotAvailable);
    });

    // Check that main thread has no errors
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

    // Wait for threads to finish
    t1.join();
    t2.join();
    t3.join();

    // Check that main thread has no errors
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::NoErrorFound);

    // Should be 500 {OutputTruncated, Warning}, 500 {OutputTruncated, Info}, 1000 {CANFDNotSupported, Warning}, 500 {CANFDSettingsNotAvailable, Info}, 500 {FailedToWrite, Info}
    EXPECT_EQ(EventCount(), 3000);
    
    auto events = GetEvents(EventFilter(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    EXPECT_EQ(EventCount(), 2500);
    EXPECT_EQ(events.size(), 500);

    events = GetEvents(EventFilter(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 2000);
    EXPECT_EQ(events.size(), 500);

    events = GetEvents(EventFilter(APIEvent::Type::CANFDNotSupported, APIEvent::Severity::EventWarning));
    EXPECT_EQ(EventCount(), 1000);
    EXPECT_EQ(events.size(), 1000);

    events = GetEvents(EventFilter(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 500);
    EXPECT_EQ(events.size(), 500);

    events = GetEvents(EventFilter(APIEvent::Type::FailedToWrite, APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 0);
    EXPECT_EQ(events.size(), 500);
}

/**
 * Checks that errors do not go into the events list, and that TooManyEvents events are not added either.
 * Checks that EventCount() updates accordingly, even when overflowing (trying to add 11000 events when the limit is 10000)
 */
TEST_F(EventManagerTest, CountTest) {
    // Add an error event, should not go into events list.
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(EventCount(), 0);

    // Adds actual event
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    
    // Manually tries to add some TooManyEvents, these should not be added.
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventWarning));
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::TooManyEvents, APIEvent::Severity::EventInfo));

    EXPECT_EQ(EventCount(), 1);

    // Add another actual event
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 2);

    // Take all info events (1)
    GetEvents(EventFilter(APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 1);

    // Take all events
    GetEvents();
    EXPECT_EQ(EventCount(), 0);

    // default limit is 10000
    for(int i = 0; i < 11000; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    
    EXPECT_EQ(EventCount(), 10000);
}

/**
 * Checks that the default get() clears out and returns all events.
 */
TEST_F(EventManagerTest, GetDefaultTest) {
    for(int i = 0; i < 5; i++) {
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::EventInfo);

        // errors should not go in events
        EventManager::GetInstance().add(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
    }

    auto events = EventManager::GetInstance().get();
    
    EXPECT_EQ(events.size(), 10);
    EXPECT_EQ(EventCount(), 0);
    
    for(int i = 0; i < 5; i++) {
        EXPECT_EQ(events.at(2 * i).getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(events.at(2 * i).getSeverity(), APIEvent::Severity::EventWarning);

        EXPECT_EQ(events.at(2 * i + 1).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
        EXPECT_EQ(events.at(2 * i + 1).getSeverity(), APIEvent::Severity::EventInfo);
    }

    // Check getting when 0 events exist doesn't break
    events = EventManager::GetInstance().get();
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks that get() with a size param only flushes and returns the desired amount, even when requesting too many.
 */
TEST_F(EventManagerTest, GetSizeTest) {

    // Add 10 events
    for(int i = 0; i < 5; i++) {
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::EventInfo);

        // errors should not go in events
        EventManager::GetInstance().add(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
    }

    // Take 3 events, 7 left
    auto events = EventManager::GetInstance().get(3);

    EXPECT_EQ(events.size(), 3);
    EXPECT_EQ(EventCount(), 7);

    EXPECT_EQ(events.at(0).getType(), APIEvent::Type::UnexpectedNetworkType);
    EXPECT_EQ(events.at(0).getSeverity(), APIEvent::Severity::EventWarning);
    EXPECT_EQ(events.at(1).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
    EXPECT_EQ(events.at(1).getSeverity(), APIEvent::Severity::EventInfo);
    EXPECT_EQ(events.at(2).getType(), APIEvent::Type::UnexpectedNetworkType);
    EXPECT_EQ(events.at(2).getSeverity(), APIEvent::Severity::EventWarning);

    // Take 1 event, 6 left
    events = EventManager::GetInstance().get(1);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(EventCount(), 6);

    EXPECT_EQ(events.at(0).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
    EXPECT_EQ(events.at(0).getSeverity(), APIEvent::Severity::EventInfo);

    // Try to take 8 events, should actually take the 6 remaining. 0 left.
    events = EventManager::GetInstance().get(8);
    EXPECT_EQ(events.size(), 6);
    EXPECT_EQ(EventCount(), 0);

    for(int i = 0; i < 3; i++) {
        EXPECT_EQ(events.at(2 * i).getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(events.at(2 * i).getSeverity(), APIEvent::Severity::EventWarning);

        EXPECT_EQ(events.at(2 * i + 1).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
        EXPECT_EQ(events.at(2 * i + 1).getSeverity(), APIEvent::Severity::EventInfo);
    }

    // Check getting when 0 events exist doesn't break
    events = EventManager::GetInstance().get(5);
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks that get() with a filter param only flushes and returns the events matching the filter.
 */
TEST_F(EventManagerTest, GetFilterTest) {
    // Add 20 events
    for(int i = 0; i < 5; i++) {
        // {network, warning}, {settings, info}, {network, info}, {mismatch, warning}
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::SettingsStructureMismatch, APIEvent::Severity::EventWarning);

        // errors should not go in events
        EventManager::GetInstance().add(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
    }

    // Get all 5 {network, warning}. 15 left.
    auto events = EventManager::GetInstance().get(EventFilter(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning));

    EXPECT_EQ(events.size(), 5);
    EXPECT_EQ(EventCount(), 15);

    for(APIEvent event : events) {
        EXPECT_EQ(event.getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(event.getSeverity(), APIEvent::Severity::EventWarning);
    }

    // Get all 10 infos. 5 {mismatch, warning} remaining.
    events = EventManager::GetInstance().get(EventFilter(APIEvent::Severity::EventInfo));

    EXPECT_EQ(events.size(), 10);
    EXPECT_EQ(EventCount(), 5);

    for(int i = 0; i < 5; i++) {
        EXPECT_EQ(events.at(2 * i).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
        EXPECT_EQ(events.at(2 * i).getSeverity(), APIEvent::Severity::EventInfo);

        EXPECT_EQ(events.at(2 * i + 1).getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(events.at(2 * i + 1).getSeverity(), APIEvent::Severity::EventInfo);
    }

    // (Incorrectly) try to get settings type again. 5 {mismatch, warning} remaining. 
    events = EventManager::GetInstance().get(EventFilter(APIEvent::Type::SWCANSettingsNotAvailable));
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 5);

    // Get the 5 {mismatch, warning} remaining.
    events = EventManager::GetInstance().get(EventFilter(APIEvent::Type::SettingsStructureMismatch));
    EXPECT_EQ(events.size(), 5);
    EXPECT_EQ(EventCount(), 0);

    for(APIEvent event : events) {
        EXPECT_EQ(event.getType(), APIEvent::Type::SettingsStructureMismatch);
        EXPECT_EQ(event.getSeverity(), APIEvent::Severity::EventWarning);
    }

    // Check getting when 0 events exist doesn't break
    events = EventManager::GetInstance().get(EventFilter(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning));
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks that get() with both a size and filter param only flushes and returns the desired amount of events matching the filter.
 */
TEST_F(EventManagerTest, GetSizeFilterTest) {
    // Add 20 events
    for(int i = 0; i < 5; i++) {
        // {network, warning}, {settings, info}, {network, info}, {mismatch, warning}
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::SettingsStructureMismatch, APIEvent::Severity::EventWarning);

        // errors should not go in events
        EventManager::GetInstance().add(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
    }

    // Get all 5 {network, warning}. 15 left.
    auto events = EventManager::GetInstance().get(6, EventFilter(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning));

    EXPECT_EQ(events.size(), 5);
    EXPECT_EQ(EventCount(), 15);

    for(APIEvent event : events) {
        EXPECT_EQ(event.getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(event.getSeverity(), APIEvent::Severity::EventWarning);
    }

    // Get 6 infos. 4 infos and 5 {mismatch, warning} remaining.
    events = EventManager::GetInstance().get(6, EventFilter(APIEvent::Severity::EventInfo));

    EXPECT_EQ(events.size(), 6);
    EXPECT_EQ(EventCount(), 9);

    for(int i = 0; i < 3; i++) {
        EXPECT_EQ(events.at(2 * i).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
        EXPECT_EQ(events.at(2 * i).getSeverity(), APIEvent::Severity::EventInfo);

        EXPECT_EQ(events.at(2 * i + 1).getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(events.at(2 * i + 1).getSeverity(), APIEvent::Severity::EventInfo);
    }

    // Get 4 remaining infos. 5 {mismatch, warning} remaining.
    events = EventManager::GetInstance().get(4, EventFilter(APIEvent::Severity::EventInfo));

    EXPECT_EQ(events.size(), 4);
    EXPECT_EQ(EventCount(), 5);

    for(int i = 0; i < 2; i++) {
        EXPECT_EQ(events.at(2 * i).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
        EXPECT_EQ(events.at(2 * i).getSeverity(), APIEvent::Severity::EventInfo);

        EXPECT_EQ(events.at(2 * i + 1).getType(), APIEvent::Type::UnexpectedNetworkType);
        EXPECT_EQ(events.at(2 * i + 1).getSeverity(), APIEvent::Severity::EventInfo);
    }

    // (Incorrectly) try to get settings type again. 5 {mismatch, warning} remaining. 
    events = EventManager::GetInstance().get(-1, EventFilter(APIEvent::Type::SWCANSettingsNotAvailable));
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 5);

    // Get the 5 {mismatch, warning} remaining.
    events = EventManager::GetInstance().get(5, EventFilter(APIEvent::Type::SettingsStructureMismatch));
    EXPECT_EQ(events.size(), 5);
    EXPECT_EQ(EventCount(), 0);

    for(APIEvent event : events) {
        EXPECT_EQ(event.getType(), APIEvent::Type::SettingsStructureMismatch);
        EXPECT_EQ(event.getSeverity(), APIEvent::Severity::EventWarning);
    }

    // Check getting when 0 events exist doesn't break
    events = EventManager::GetInstance().get(2, EventFilter(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::EventWarning));
    EXPECT_EQ(events.size(), 0);
    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks that adding 1 error and calling GetLastError() twice will first return the error then return a NoErrorFound info message. Singlethreaded.
 */
TEST_F(EventManagerTest, GetLastErrorSingleTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
    auto err = GetLastError();
    EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
    EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
}

/**
 * Checks that adding multiple errors and calling GetLastError() twice will first return the last error then return a NoErrorFound info message. Singlethreaded.
 */
TEST_F(EventManagerTest, GetLastErrorMultipleTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error));
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error));
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::SettingsNotAvailable);
    auto err = GetLastError();
    EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
    EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
}

/**
 * Adds 52 events when the limit is 50 (49 normal, 1 reserved)
 * Checks that only the latest 49 are kept, and a TooManyEvents warning exists at the end.
 */
TEST_F(EventManagerTest, TestAddWarningsOverflow) {
    
    // space for 49 normal events, 1 reserved for TooManyEvents
    SetEventLimit(50);

    // 3 of these
    for(int i = 0; i < 3; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));

    // 1 info
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::EventInfo));

    // 48 of these
    for(int i = 0; i < 48; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::EventWarning));

    auto events = GetEvents();

    EXPECT_EQ(events.at(0).getType(), APIEvent::Type::SWCANSettingsNotAvailable);
    EXPECT_EQ(events.at(0).getSeverity(), APIEvent::Severity::EventInfo);

    for(int i = 1; i < 49; i++) {
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::ParameterOutOfRange);
        EXPECT_EQ(events.at(i).getSeverity(), APIEvent::Severity::EventWarning);
    }

    EXPECT_EQ(events.at(49).getType(), APIEvent::Type::TooManyEvents);
    EXPECT_EQ(events.at(49).getSeverity(), APIEvent::Severity::EventWarning);
}

/**
 * Adds 1 warning, 3 info, and 47 warning events, in that order, when the limit is 50 (49 normal, 1 reserved)
 * Checks that only the latest 49 are kept, and a TOoManyEvents warning exists at the end.
 */
TEST_F(EventManagerTest, TestAddWarningsInfoOverflow) {
    
    // space for 49 normal events, 1 reserved for TooManyEvents
    SetEventLimit(50);

    // Event list filling: 1 warning, 3 info, 47 warning.
    // Expect to see: 2 info, 47 warning, 1 TooManyEvents

    EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsVersionError, APIEvent::Severity::EventWarning));

    // 3 of these
    for(int i = 0; i < 3; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));

    // 47 of these
    for(int i = 0; i < 47; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::EventWarning));

    auto events = GetEvents();

    for(int i = 0; i < 2; i++) {
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::OutputTruncated);
    }

    for(int i = 2; i < 49; i++)
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::ParameterOutOfRange);
    
    EXPECT_EQ(events.at(49).getType(), APIEvent::Type::TooManyEvents);
}

/**
 * Checks that discarding with no params flushes every event.
 */
TEST_F(EventManagerTest, DiscardDefault) {
    for(int i = 0; i < 3000; i++) {
        EventManager::GetInstance().add(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::BufferInsufficient, APIEvent::Severity::EventWarning);
    }

    EXPECT_EQ(EventCount(), 9000);

    DiscardEvents();

    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks that discarding with a filter only flushes events matching the filter.
 */
TEST_F(EventManagerTest, DiscardFilter) {
    for(int i = 0; i < 3000; i++) {
        EventManager::GetInstance().add(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo);
        EventManager::GetInstance().add(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventWarning);
        EventManager::GetInstance().add(APIEvent::Type::BufferInsufficient, APIEvent::Severity::EventWarning);
    }

    EXPECT_EQ(EventCount(), 9000);

    DiscardEvents(EventFilter(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::EventInfo));

    EXPECT_EQ(EventCount(), 6000);

    DiscardEvents(EventFilter(APIEvent::Type::BufferInsufficient, APIEvent::Severity::EventInfo));

    EXPECT_EQ(EventCount(), 6000);

    DiscardEvents(EventFilter(APIEvent::Severity::EventWarning));

    EXPECT_EQ(EventCount(), 0);
}

/**
 * Checks setting the event limit when truncating is not required, when the new limit is < 10, and when the new limit < num events.
 */
TEST_F(EventManagerTest, SetEventLimitTest) {
    // Test if event limit too low to be set
    EventManager::GetInstance().setEventLimit(9);
    EXPECT_EQ(GetEventLimit(), 10000);
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::ParameterOutOfRange);

    // Test truncating existing list when new limit set
    for(int i = 0; i < 9001; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    
    EXPECT_EQ(EventCount(), 9001);

    // Sets new limit to be exactly full.
    SetEventLimit(9002);
    EXPECT_EQ(GetEventLimit(), 9002);
    EXPECT_EQ(EventCount(), 9001);
    
    // 1 overflowed.
    SetEventLimit(9001);
    EXPECT_EQ(GetEventLimit(), 9001);
    EXPECT_EQ(EventCount(), 9001);

    // Truncate a lot
    SetEventLimit(5000);
    EXPECT_EQ(GetEventLimit(), 5000);
    EXPECT_EQ(EventCount(), 5000);

    auto events = GetEvents();
    for(int i = 0; i < 4998; i++) {
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::OutputTruncated);
    }
    EXPECT_EQ(events.at(4999).getType(), APIEvent::Type::TooManyEvents);
}