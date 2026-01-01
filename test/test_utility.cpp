/**
 * @file test_utility.cpp
 * @brief Unit tests for utility functions
 */

#include "CppUTest/TestHarness.h"
#include "utility.h"
#include <stdint.h>
#include <limits.h>

TEST_GROUP(CalculateTickDifference_NormalOperation)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(CalculateTickDifference_NormalOperation, NoTimePassed)
{
    uint32_t diff = CalculateTickDifference(1000, 1000);
    LONGS_EQUAL(0, diff);
}

TEST(CalculateTickDifference_NormalOperation, OneTickPassed)
{
    uint32_t diff = CalculateTickDifference(1001, 1000);
    LONGS_EQUAL(1, diff);
}

TEST(CalculateTickDifference_NormalOperation, TenTicksPassed)
{
    uint32_t diff = CalculateTickDifference(1010, 1000);
    LONGS_EQUAL(10, diff);
}

TEST(CalculateTickDifference_NormalOperation, HundredTicksPassed)
{
    uint32_t diff = CalculateTickDifference(1100, 1000);
    LONGS_EQUAL(100, diff);
}

TEST(CalculateTickDifference_NormalOperation, ThousandTicksPassed)
{
    uint32_t diff = CalculateTickDifference(2000, 1000);
    LONGS_EQUAL(1000, diff);
}

TEST(CalculateTickDifference_NormalOperation, LargeNormalDifference)
{
    uint32_t diff = CalculateTickDifference(1000000, 500000);
    LONGS_EQUAL(500000, diff);
}

TEST_GROUP(CalculateTickDifference_Overflow)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(CalculateTickDifference_Overflow, OverflowByOne)
{
    // Last tick was at max value, now it wrapped to 0
    uint32_t diff = CalculateTickDifference(0, UINT32_MAX);
    LONGS_EQUAL(1, diff);
}

TEST(CalculateTickDifference_Overflow, OverflowByTen)
{
    // Last tick was 10 before overflow
    uint32_t diff = CalculateTickDifference(9, UINT32_MAX);
    LONGS_EQUAL(10, diff);
}

TEST(CalculateTickDifference_Overflow, OverflowByHundred)
{
    // Last tick was 100 before overflow
    uint32_t diff = CalculateTickDifference(99, UINT32_MAX);
    LONGS_EQUAL(100, diff);
}

TEST(CalculateTickDifference_Overflow, OverflowFromNearMax)
{
    // Last tick was 5 before max
    uint32_t diff = CalculateTickDifference(5, UINT32_MAX - 5);
    LONGS_EQUAL(11, diff);
}

TEST(CalculateTickDifference_Overflow, OverflowToLargeValue)
{
    // Wrap from near max to a larger value after overflow
    uint32_t diff = CalculateTickDifference(1000, UINT32_MAX - 999);
    LONGS_EQUAL(2000, diff);
}

TEST(CalculateTickDifference_Overflow, LargeGapAcrossOverflow)
{
    // Large time difference spanning the overflow boundary
    uint32_t diff = CalculateTickDifference(5000, UINT32_MAX - 5000);
    LONGS_EQUAL(10001, diff);
}

TEST(CalculateTickDifference_Overflow, MultipleMillionTicksAcrossOverflow)
{
    // Simulate several million ticks across overflow
    uint32_t diff = CalculateTickDifference(1000000, UINT32_MAX - 1000000);
    LONGS_EQUAL(2000001, diff);
}

TEST_GROUP(CalculateTickDifference_EdgeCases)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(CalculateTickDifference_EdgeCases, BothZero)
{
    uint32_t diff = CalculateTickDifference(0, 0);
    LONGS_EQUAL(0, diff);
}

TEST(CalculateTickDifference_EdgeCases, FromZeroToOne)
{
    uint32_t diff = CalculateTickDifference(1, 0);
    LONGS_EQUAL(1, diff);
}

TEST(CalculateTickDifference_EdgeCases, BothMaxValue)
{
    uint32_t diff = CalculateTickDifference(UINT32_MAX, UINT32_MAX);
    LONGS_EQUAL(0, diff);
}

TEST(CalculateTickDifference_EdgeCases, MaxMinusOneToMax)
{
    uint32_t diff = CalculateTickDifference(UINT32_MAX, UINT32_MAX - 1);
    LONGS_EQUAL(1, diff);
}

TEST(CalculateTickDifference_EdgeCases, FromZeroToMax)
{
    uint32_t diff = CalculateTickDifference(UINT32_MAX, 0);
    LONGS_EQUAL(UINT32_MAX, diff);
}

TEST(CalculateTickDifference_EdgeCases, NearCompleteWrap)
{
    // Current tick is just before last tick (almost complete wrap)
    uint32_t diff = CalculateTickDifference(1000, 1001);
    LONGS_EQUAL(UINT32_MAX, diff);
}

TEST(CalculateTickDifference_EdgeCases, CompleteWrapMinusOne)
{
    // Exactly one less than a complete wrap
    uint32_t diff = CalculateTickDifference(999, 1000);
    LONGS_EQUAL(UINT32_MAX, diff);
}

TEST_GROUP(CalculateTickDifference_Sequence)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(CalculateTickDifference_Sequence, SequentialNormalCalls)
{
    uint32_t last = 1000;
    uint32_t current;
    uint32_t diff;
    
    // First increment
    current = 1001;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(1, diff);
    last = current;
    
    // Second increment
    current = 1010;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(9, diff);
    last = current;
    
    // Third increment
    current = 1100;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(90, diff);
}

TEST(CalculateTickDifference_Sequence, SequenceThroughOverflow)
{
    uint32_t last = UINT32_MAX - 5;
    uint32_t current;
    uint32_t diff;
    
    // Increment towards max
    current = UINT32_MAX - 3;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(2, diff);
    last = current;
    
    // Reach max
    current = UINT32_MAX;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(3, diff);
    last = current;
    
    // Overflow to 0
    current = 0;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(1, diff);
    last = current;
    
    // Continue after overflow
    current = 10;
    diff = CalculateTickDifference(current, last);
    LONGS_EQUAL(10, diff);
}

TEST(CalculateTickDifference_Sequence, RepeatedSmallIncrements)
{
    uint32_t last = 0;
    uint32_t current;
    
    for (int i = 1; i <= 100; i++)
    {
        current = i;
        uint32_t diff = CalculateTickDifference(current, last);
        LONGS_EQUAL(1, diff);
        last = current;
    }
}
