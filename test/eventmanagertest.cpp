#include <thread>

#include "icsneo/icsneocpp.h"
#include "gtest/gtest.h"

using namespace icsneo;

class EventManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        EventManager::ResetInstance();
    }
};

// Tests that adding and removing events properly updates EventCount(). Also tests that EventCount() does not go past the limit.
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

// Test default get params
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

// Test get with a size limit
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

// Test get with a filter (type, severity)
// Doesn't test with device!
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

// Test get with both size limit and filter
// Doesn't test with devices
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

// Multithreaded test of GetLastError()
TEST_F(EventManagerTest, GetLastErrorMultiThreaded) {
    std::thread t1( []() {
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
        auto err = GetLastError();
        EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
        EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
    });

    std::thread t2( []() {
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error));
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error));
        EXPECT_EQ(GetLastError().getType(), APIEvent::Type::SettingsNotAvailable);
        auto err = GetLastError();
        EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
        EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
    });

    t1.join();
    t2.join();
}

// Tests that adding 1 error and calling GetLastError() twice will first return the error then return a NoErrorFound info message. Singlethreaded.
TEST_F(EventManagerTest, GetLastErrorSingleTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
    auto err = GetLastError();
    EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
    EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
}

// Tests that adding multiple errors and calling GetLastError() twice will first return the last error then return a NoErrorFound info message. Singlethreaded.
TEST_F(EventManagerTest, GetLastErrorMultipleTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error));
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error));
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::SettingsNotAvailable);
    auto err = GetLastError();
    EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
    EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
}

// Tests the case where too many warnings are added
TEST_F(EventManagerTest, TestAddWarningsOverflow) {
    
    // space for 49 normal events, 1 reserved for TooManyEvents
    SetEventLimit(50);

    // 3 of these
    for(int i = 0; i < 3; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));

    // 49 of these
    for(int i = 0; i < 49; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::EventWarning));

    auto events = GetEvents();
    for(int i = 0; i < 49; i++)
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::ParameterOutOfRange);
    
    EXPECT_EQ(events.at(49).getType(), APIEvent::Type::TooManyEvents);
}

// Tests the case where too many warnings and info are added
TEST_F(EventManagerTest, TestAddWarningsInfoOverflow) {
    
    // space for 49 normal events, 1 reserved for TooManyEvents
    SetEventLimit(50);

    // Event list filling: 1 warning, 3 info, 47 warning.
    // Expect to see: 1 warning, 1 info, 47 warning, 1 TooManyEvents

    EventManager::GetInstance().add(APIEvent(APIEvent::Type::SettingsVersionError, APIEvent::Severity::EventWarning));

    // 3 of these
    for(int i = 0; i < 3; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));

    // 47 of these
    for(int i = 0; i < 47; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::EventWarning));

    auto events = GetEvents();

    EXPECT_EQ(events.at(0).getType(), APIEvent::Type::SettingsVersionError);

    EXPECT_EQ(events.at(1).getType(), APIEvent::Type::OutputTruncated);

    for(int i = 2; i < 49; i++)
        EXPECT_EQ(events.at(i).getType(), APIEvent::Type::ParameterOutOfRange);
    
    EXPECT_EQ(events.at(49).getType(), APIEvent::Type::TooManyEvents);
}

// Tests that setting the event limit works in normal conditions, if the new limit is too small, and if the list needs truncating
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

    // auto events = GetEvents();
    // for(int i = 0; i < 4998; i++) {
    //     EXPECT_EQ(events.at(i).getType(), APIEvent::Type::OutputTruncated);
    // }
    // EXPECT_EQ(events.at(4999).getType(), APIEvent::Type::TooManyEvents);
}

// Tests that setting the event limit when already overflowing works
TEST_F(EventManagerTest, SetEventLimitOverflowTest) {

}