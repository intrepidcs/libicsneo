#include "icsneo/core/ringbuffer.h"
#include "gtest/gtest.h"

using namespace icsneo;

class RingBufferTest : public ::testing::Test {
protected:
    static constexpr const size_t bufferSize = 32u;
    static constexpr const size_t testDataSize = 32u;
    RingBuffer ringBuffer = RingBuffer(bufferSize);
    void SetUp() override {
        ringBuffer.clear();
    }

    const std::vector<uint8_t> testBytes = { 
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31
    };
};

TEST_F(RingBufferTest, ConstructorTest) {
    // Standard case, integral power of 2
    ASSERT_EQ(RingBuffer(16).capacity(), 16u);
    // Edge cases
    // SIZE_MAX - commented out because this will throw on all architectures due to the allocation that happens here.
    //ASSERT_EQ(RingBuffer(SIZE_MAX).capacity(), RingBuffer::MaxSize);
    // Zero
    ASSERT_EQ(RingBuffer(0).capacity(), 1u);
    // arbitrary number that is not a power of 2
    ASSERT_EQ(RingBuffer(60).capacity(), 64u);
}

TEST_F(RingBufferTest, InitAndCapacityTest) {
    constexpr auto size = 8u;
    RingBuffer rb(size);
    ASSERT_EQ(rb.size(), 0u);
    ASSERT_EQ(rb.capacity(), size);
}

TEST_F(RingBufferTest, WriteAndClearTest) {
    ASSERT_TRUE(ringBuffer.write(testBytes));
    ASSERT_EQ(ringBuffer.size(), testBytes.size());
    ringBuffer.clear();
    ASSERT_EQ(ringBuffer.size(), 0u);
}

TEST_F(RingBufferTest, SimpleWriteReadTest) {
    std::vector<uint8_t> readBack(testDataSize);
    ASSERT_TRUE(ringBuffer.write(testBytes));
    ASSERT_EQ(ringBuffer.size(), testDataSize);
    ASSERT_TRUE(ringBuffer.read(readBack.data(), 0, testDataSize));
    ASSERT_EQ(readBack, testBytes);
}

TEST_F(RingBufferTest, OverlappedReadWriteTest) {
    std::vector<uint8_t> readBack(testDataSize);
    std::vector<uint8_t> ignoredData(bufferSize - 3);
    ASSERT_TRUE(ringBuffer.write(ignoredData));
    ringBuffer.pop(ignoredData.size());
    ASSERT_EQ(ringBuffer.size(), 0u);
    ASSERT_TRUE(ringBuffer.write(testBytes));
    ASSERT_TRUE(ringBuffer.read(readBack.data(), 0, testDataSize));
    ASSERT_EQ(readBack, testBytes);
}

TEST_F(RingBufferTest, WritePastReadCursorTest) {
    std::vector<uint8_t> readBack(ringBuffer.capacity());
    // Fill
    ASSERT_TRUE(ringBuffer.write(testBytes));
    // Read partial
    auto readSize = ringBuffer.size() - 4;
    ASSERT_TRUE(ringBuffer.read(readBack.data(), 0, readSize));
    // Now writeCursor (masked) is 0, readCursor (masked) is capacity() - 4, writing past the read cursor should fail.
    ASSERT_FALSE(ringBuffer.write(testBytes.data(), readSize + 1));
    
}

TEST_F(RingBufferTest, WriteWhenFullTest) {
    std::vector<uint8_t> fillData(ringBuffer.capacity());
    ASSERT_TRUE(ringBuffer.write(fillData));
    ASSERT_FALSE(ringBuffer.write(fillData.data(), 1));
}

TEST_F(RingBufferTest, ReadPastEndTest) {
    uint8_t dummy = 0;
    // Single byte when empty
    ASSERT_FALSE(ringBuffer.read(&dummy, 0, 1));
    // Put in a byte
    ASSERT_TRUE(ringBuffer.write(&dummy, 1));
    // Single byte from offset when filled only to offset
    ASSERT_FALSE(ringBuffer.read(&dummy, ringBuffer.size(), 1));
    // Single byte from offset past size
    ASSERT_FALSE(ringBuffer.read(&dummy, ringBuffer.size()+1, 1));
}
