#include "icsneo/icsneocpp.h"
#include "gtest/gtest.h"

using namespace icsneo;

class EventManagerTest : public ::testing::Test {
protected:
    void TearDown() override {
        EventManager::ResetInstance();
    }
};

TEST_F(EventManagerTest, GetLastErrorTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(GetLastError().getType(), APIEvent::Type::OutputTruncated);
    auto err = GetLastError();
    EXPECT_EQ(err.getType(), APIEvent::Type::NoErrorFound);
    EXPECT_EQ(err.getSeverity(), APIEvent::Severity::EventInfo);
}

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
}