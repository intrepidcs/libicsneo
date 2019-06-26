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

// Tests that adding and removing events properly updates EventCount(). Also tests that EventCount() does not go past the limit.
TEST_F(EventManagerTest, CountTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(EventCount(), 0);
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    EXPECT_EQ(EventCount(), 1);
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 2);
    GetEvents(EventFilter(APIEvent::Severity::EventInfo));
    EXPECT_EQ(EventCount(), 1);
    GetEvents();
    EXPECT_EQ(EventCount(), 0);

    SetEventLimit(50);
    for(int i = 0; i < 60; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    
    EXPECT_EQ(EventCount(), 50);
}

// 
TEST_F(EventManagerTest, GetTest) {

}

// Tests that setting the event limit works in normal conditions, if the new limit is too small, and if the list needs truncating
TEST_F(EventManagerTest, SetEventLimitTest) {
    // Test if event limit too low to be set
    EventManager::GetInstance().setEventLimit(9);
    EXPECT_EQ(GetEventLimit(), 10000);
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::ParameterOutOfRange);

    // Test truncating existing list when new limit set
    for(int i = 0; i < 100; i++)
        EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::EventWarning));
    
    EXPECT_EQ(EventCount(), 100);

    SetEventLimit(50);
    EXPECT_EQ(GetEventLimit(), 50);

    // maybe 5001? since TooManyEvents is in there too.
    EXPECT_EQ(EventCount(), 50);
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