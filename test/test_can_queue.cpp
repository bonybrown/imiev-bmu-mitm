/**
 * @file test_can_queue.cpp
 * @brief Unit tests for CanQueue class
 */

#include "CppUTest/TestHarness.h"
#include "CanQueue.h"
#include <string.h>

TEST_GROUP(CanQueue_Construction){
    void setup(){}

    void teardown(){}};

TEST(CanQueue_Construction, DefaultConstructor)
{
    CanQueue<16> queue;

    CHECK(queue.isEmpty());
    CHECK_FALSE(queue.isFull());
    LONGS_EQUAL(0, queue.length());
    LONGS_EQUAL(16, queue.capacity());
    LONGS_EQUAL(16, queue.available());
}

TEST(CanQueue_Construction, DifferentSizes)
{
    CanQueue<8> small;
    CanQueue<32> medium;
    CanQueue<64> large;

    LONGS_EQUAL(8, small.capacity());
    LONGS_EQUAL(32, medium.capacity());
    LONGS_EQUAL(64, large.capacity());
}

TEST_GROUP(CanQueue_BasicOperations)
{
    CanQueue<16> queue;
    CAN_FRAME frame;

    void setup()
    {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x123;
        frame.dlc = 8;
    }

    void teardown()
    {
    }
};

TEST(CanQueue_BasicOperations, PushOneFrame)
{
    bool result = queue.push(frame);

    CHECK(result);
    CHECK_FALSE(queue.isEmpty());
    LONGS_EQUAL(1, queue.length());
    LONGS_EQUAL(15, queue.available());
}

TEST(CanQueue_BasicOperations, PopOneFrame)
{
    queue.push(frame);

    CAN_FRAME popped;
    bool result = queue.pop(&popped);

    CHECK(result);
    CHECK(queue.isEmpty());
    LONGS_EQUAL(0, queue.length());
    LONGS_EQUAL(0x123, popped.ID);
    LONGS_EQUAL(8, popped.dlc);
}

TEST(CanQueue_BasicOperations, PopEmptyQueue)
{
    CAN_FRAME popped;
    bool result = queue.pop(&popped);

    CHECK_FALSE(result);
    CHECK(queue.isEmpty());
}

TEST(CanQueue_BasicOperations, PopWithNullPointer)
{
    queue.push(frame);

    bool result = queue.pop(nullptr);

    CHECK(result); // Should succeed even with null pointer
    CHECK(queue.isEmpty());
}

TEST(CanQueue_BasicOperations, PeekFrame)
{
    frame.ID = 0x456;
    queue.push(frame);

    CAN_FRAME peeked;
    bool result = queue.peek(&peeked);

    CHECK(result);
    LONGS_EQUAL(0x456, peeked.ID);
    LONGS_EQUAL(1, queue.length()); // Peek should not remove
}

TEST(CanQueue_BasicOperations, PeekEmptyQueue)
{
    CAN_FRAME peeked;
    bool result = queue.peek(&peeked);

    CHECK_FALSE(result);
}

TEST(CanQueue_BasicOperations, PushPopSequence)
{
    frame.ID = 0x111;
    queue.push(frame);

    frame.ID = 0x222;
    queue.push(frame);

    frame.ID = 0x333;
    queue.push(frame);

    LONGS_EQUAL(3, queue.length());

    CAN_FRAME popped;
    queue.pop(&popped);
    LONGS_EQUAL(0x111, popped.ID);

    queue.pop(&popped);
    LONGS_EQUAL(0x222, popped.ID);

    queue.pop(&popped);
    LONGS_EQUAL(0x333, popped.ID);

    CHECK(queue.isEmpty());
}

TEST_GROUP(CanQueue_CapacityHandling)
{
    CanQueue<4> queue; // Small queue for testing
    CAN_FRAME frame;

    void setup()
    {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x100;
        frame.dlc = 8;
    }

    void teardown()
    {
    }
};

TEST(CanQueue_CapacityHandling, FillQueue)
{
    for (int i = 0; i < 4; i++)
    {
        frame.ID = 0x100 + i;
        bool result = queue.push(frame);
        CHECK(result);
    }

    CHECK(queue.isFull());
    LONGS_EQUAL(4, queue.length());
    LONGS_EQUAL(0, queue.available());
}

TEST(CanQueue_CapacityHandling, OverflowPrevention)
{
    // Fill the queue
    for (int i = 0; i < 4; i++)
    {
        queue.push(frame);
    }

    // Try to push one more
    frame.ID = 0x999;
    bool result = queue.push(frame);

    CHECK_FALSE(result); // Should fail
    CHECK(queue.isFull());
    LONGS_EQUAL(4, queue.length());
}

TEST(CanQueue_CapacityHandling, FillEmptyFill)
{
    // Fill queue
    for (int i = 0; i < 4; i++)
    {
        frame.ID = 0x100 + i;
        queue.push(frame);
    }

    // Empty queue
    CAN_FRAME popped;
    for (int i = 0; i < 4; i++)
    {
        queue.pop(&popped);
    }

    CHECK(queue.isEmpty());

    // Fill again
    for (int i = 0; i < 4; i++)
    {
        frame.ID = 0x200 + i;
        bool result = queue.push(frame);
        CHECK(result);
    }

    CHECK(queue.isFull());
    LONGS_EQUAL(4, queue.length());
}

TEST_GROUP(CanQueue_CircularBehavior)
{
    CanQueue<4> queue;
    CAN_FRAME frame;

    void setup()
    {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.dlc = 8;
    }

    void teardown()
    {
    }
};

TEST(CanQueue_CircularBehavior, WrapAroundHead)
{
    // Fill and partially empty to move head/tail
    for (int i = 0; i < 4; i++)
    {
        frame.ID = 0x100 + i;
        queue.push(frame);
    }

    CAN_FRAME popped;
    queue.pop(&popped); // Remove one
    queue.pop(&popped); // Remove another

    // Add two more (should wrap around)
    frame.ID = 0x500;
    CHECK(queue.push(frame));
    frame.ID = 0x600;
    CHECK(queue.push(frame));

    CHECK(queue.isFull());
}

TEST(CanQueue_CircularBehavior, OrderPreservation)
{
    // Add some frames
    for (uint32_t i = 0; i < 4; i++)
    {
        frame.ID = 0x100 + i;
        queue.push(frame);
    }

    // Remove and verify FIFO order
    CAN_FRAME popped;
    for (uint32_t i = 0; i < 4; i++)
    {
        queue.pop(&popped);
        LONGS_EQUAL(0x100 + i, popped.ID);
    }
}

TEST_GROUP(CanQueue_DataIntegrity)
{
    CanQueue<16> queue;

    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(CanQueue_DataIntegrity, FullFrameData)
{
    CAN_FRAME frame;
    frame.ID = 0x7FF;
    frame.dlc = 8;
    frame.ide = 0;
    frame.rtr = 0;
    frame.tx_channel = 0;
    for (int i = 0; i < 8; i++)
    {
        frame.data[i] = i * 10;
    }

    queue.push(frame);

    CAN_FRAME popped;
    queue.pop(&popped);

    LONGS_EQUAL(0x7FF, popped.ID);
    LONGS_EQUAL(8, popped.dlc);
    LONGS_EQUAL(0, popped.ide);
    LONGS_EQUAL(0, popped.rtr);
    LONGS_EQUAL(0, popped.tx_channel);

    for (int i = 0; i < 8; i++)
    {
        LONGS_EQUAL(i * 10, popped.data[i]);
    }
}

TEST(CanQueue_DataIntegrity, ExtendedID)
{
    CAN_FRAME frame;
    frame.ID = 0x1FFFFFFF; // 29-bit extended ID
    frame.dlc = 4;
    frame.ide = 1;
    frame.rtr = 0;
    frame.data[0] = 0xAA;
    frame.data[1] = 0xBB;
    frame.data[2] = 0xCC;
    frame.data[3] = 0xDD;

    queue.push(frame);

    CAN_FRAME popped;
    queue.pop(&popped);

    LONGS_EQUAL(0x1FFFFFFF, popped.ID);
    LONGS_EQUAL(4, popped.dlc);
    LONGS_EQUAL(1, popped.ide);
    LONGS_EQUAL(0xAA, popped.data[0]);
    LONGS_EQUAL(0xBB, popped.data[1]);
    LONGS_EQUAL(0xCC, popped.data[2]);
    LONGS_EQUAL(0xDD, popped.data[3]);
}

TEST(CanQueue_DataIntegrity, RemoteFrame)
{
    CAN_FRAME frame;
    frame.ID = 0x400;
    frame.dlc = 0;
    frame.ide = 0;
    frame.rtr = 1; // Remote transmission request

    queue.push(frame);

    CAN_FRAME popped;
    queue.pop(&popped);

    LONGS_EQUAL(0x400, popped.ID);
    LONGS_EQUAL(0, popped.dlc);
    LONGS_EQUAL(1, popped.rtr);
}

TEST(CanQueue_DataIntegrity, MultipleFramesDifferentData)
{
    // Push multiple frames with different data
    for (int i = 0; i < 5; i++)
    {
        CAN_FRAME frame;
        frame.ID = 0x300 + i;
        frame.dlc = i + 1;
        frame.ide = i % 2;
        frame.rtr = 0;

        for (int j = 0; j < frame.dlc; j++)
        {
            frame.data[j] = i * 10 + j;
        }

        queue.push(frame);
    }

    // Pop and verify each frame
    for (int i = 0; i < 5; i++)
    {
        CAN_FRAME popped;
        queue.pop(&popped);

        LONGS_EQUAL(0x300 + i, popped.ID);
        LONGS_EQUAL(i + 1, popped.dlc);
        LONGS_EQUAL(i % 2, popped.ide);

        for (int j = 0; j < popped.dlc; j++)
        {
            LONGS_EQUAL(i * 10 + j, popped.data[j]);
        }
    }
}

TEST_GROUP(CanQueue_ClearOperation)
{
    CanQueue<8> queue;
    CAN_FRAME frame;

    void setup()
    {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x100;
        frame.dlc = 8;
    }

    void teardown()
    {
    }
};

TEST(CanQueue_ClearOperation, ClearEmptyQueue)
{
    queue.clear();

    CHECK(queue.isEmpty());
    LONGS_EQUAL(0, queue.length());
}

TEST(CanQueue_ClearOperation, ClearPartiallyFilledQueue)
{
    // Add some frames
    for (int i = 0; i < 3; i++)
    {
        queue.push(frame);
    }

    LONGS_EQUAL(3, queue.length());

    queue.clear();

    CHECK(queue.isEmpty());
    LONGS_EQUAL(0, queue.length());
    LONGS_EQUAL(8, queue.available());
}

TEST(CanQueue_ClearOperation, ClearFullQueue)
{
    // Fill queue
    for (int i = 0; i < 8; i++)
    {
        queue.push(frame);
    }

    CHECK(queue.isFull());

    queue.clear();

    CHECK(queue.isEmpty());
    LONGS_EQUAL(0, queue.length());
}

TEST(CanQueue_ClearOperation, UseAfterClear)
{
    // Fill, clear, then use again
    for (int i = 0; i < 5; i++)
    {
        queue.push(frame);
    }

    queue.clear();

    // Should be able to use normally after clear
    frame.ID = 0x999;
    CHECK(queue.push(frame));

    CAN_FRAME popped;
    CHECK(queue.pop(&popped));
    LONGS_EQUAL(0x999, popped.ID);
}

TEST_GROUP(CanQueue_RealWorldScenarios){
    void setup(){}

    void teardown(){}};

TEST(CanQueue_RealWorldScenarios, BurstTraffic)
{
    CanQueue<32> queue;
    CAN_FRAME frame;

    // Simulate burst of CAN messages
    for (int i = 0; i < 20; i++)
    {
        frame.ID = 0x100 + i;
        frame.dlc = 8;
        frame.ide = 0;
        frame.rtr = 0;

        for (int j = 0; j < 8; j++)
        {
            frame.data[j] = i + j;
        }

        CHECK(queue.push(frame));
    }

    LONGS_EQUAL(20, queue.length());
    CHECK_FALSE(queue.isFull());
}

TEST(CanQueue_RealWorldScenarios, ProducerConsumerPattern)
{
    CanQueue<16> queue;
    CAN_FRAME frame;
    int produced = 0;
    int consumed = 0;

    // Simulate producer/consumer with uneven rates
    for (int cycle = 0; cycle < 50; cycle++)
    {
        // Producer adds 2 frames
        for (int i = 0; i < 2; i++)
        {
            if (!queue.isFull())
            {
                frame.ID = 0x300 + produced;
                frame.dlc = 8;
                queue.push(frame);
                produced++;
            }
        }

        // Consumer removes 1 frame
        if (!queue.isEmpty())
        {
            CAN_FRAME popped;
            queue.pop(&popped);
            consumed++;
        }
    }

    // Should have some frames queued
    LONGS_EQUAL(produced - consumed, queue.length());
}

TEST_GROUP(CanQueue_MemoryHandling){
    void setup(){}

    void teardown(){}};

TEST(CanQueue_MemoryHandling, TakesCopyOfFrame)
{
    CanQueue<16> queue;
    CAN_FRAME frame;
    frame.ID = 0x123;
    frame.dlc = 8;
    queue.push(frame);
    // Modify original frame
    frame.ID = 0x999;
    LONGS_EQUAL(0x999, frame.ID);
    // Pop from queue
    CAN_FRAME popped;
    queue.pop(&popped);
    // Verify popped frame is unchanged
    LONGS_EQUAL(0x123, popped.ID);
}