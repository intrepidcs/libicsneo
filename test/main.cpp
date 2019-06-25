#include "icsneo/api/eventmanager.h"
#include "gtest/gtest.h"

using namespace icsneo;

class EventManagerTest : public ::testing::Test {

};

TEST_F(EventManagerTest, GetLastErrorTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(EventManager::GetInstance().getLastError().getType(), APIEvent::Type::OutputTruncated);
}

TEST_F(EventManagerTest, CountTest) {
    EventManager::GetInstance().add(APIEvent(APIEvent::Type::OutputTruncated, APIEvent::Severity::Error));
    EXPECT_EQ(EventManager::GetInstance().count(), 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}