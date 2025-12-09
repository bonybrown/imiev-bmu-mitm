/**
 * @file test_can_bridge.cpp
 * @brief Unit tests for CAN bridge firmware functions
 */

#include "CppUTest/TestHarness.h"

extern "C" {
    #include <stdint.h>
    #include <string.h>
    #include "../Src/conversion.h"
}

TEST_GROUP(VoltageConversion)
{
    void setup() {
        // Setup code before each test
    }
    
    void teardown() {
        // Cleanup code after each test
    }
};

TEST(VoltageConversion, MinimumVoltage)
{
    // Test minimum voltage conversion (2.75V)
    uint8_t result = convert_voltage_to_char(2.75f);
    LONGS_EQUAL(65, result); // 2.75 * 100 - 210 = 65
}

TEST(VoltageConversion, MaximumVoltage)
{
    // Test maximum voltage conversion (4.2V)
    // 4.2 * 100 - 210 = 210, but due to float precision might be 209
    uint8_t result = convert_voltage_to_char(4.2f);
    CHECK(result >= 209 && result <= 210); // Accept either due to float precision
}

TEST(VoltageConversion, NominalVoltage)
{
    // Test nominal voltage (3.7V)
    uint8_t result = convert_voltage_to_char(3.7f);
    LONGS_EQUAL(160, result); // 3.7 * 100 - 210 = 160
}

TEST_GROUP(SoCCalculations)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(SoCCalculations, ClampMinimum)
{
    // Test SoC clamping at minimum
    float result = clamp_soc(-10.0f);
    DOUBLES_EQUAL(0.0f, result, 0.01);
}

TEST(SoCCalculations, ClampMaximum)
{
    // Test SoC clamping at maximum
    float result = clamp_soc(150.0f);
    DOUBLES_EQUAL(100.0f, result, 0.01);
}

TEST(SoCCalculations, ClampNormal)
{
    // Test SoC within normal range
    float result = clamp_soc(55.5f);
    DOUBLES_EQUAL(55.5f, result, 0.01);
}

TEST_GROUP(BatteryCapacity)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(BatteryCapacity, FullCapacity)
{
    // Test with no consumption
    float result = calculate_remaining_capacity(90.0f, 0.0f);
    DOUBLES_EQUAL(90.0f, result, 0.01);
}

TEST(BatteryCapacity, HalfCapacity)
{
    // Test with 50% consumption
    float result = calculate_remaining_capacity(90.0f, 45.0f);
    DOUBLES_EQUAL(45.0f, result, 0.01);
}

TEST(BatteryCapacity, OverConsumption)
{
    // Test with over-consumption (should clamp to 0)
    float result = calculate_remaining_capacity(90.0f, 100.0f);
    DOUBLES_EQUAL(0.0f, result, 0.01);
}

TEST(BatteryCapacity, NearEmpty)
{
    // Test near empty condition
    float result = calculate_remaining_capacity(90.0f, 89.5f);
    DOUBLES_EQUAL(0.5f, result, 0.01);
}

TEST_GROUP(MemoryOperations)
{
    uint8_t buffer[8];
    
    void setup() {
        memset(buffer, 0, sizeof(buffer));
    }
    
    void teardown() {
    }
};

TEST(MemoryOperations, BufferInitialization)
{
    // Test that buffer is properly initialized
    for (int i = 0; i < 8; i++) {
        LONGS_EQUAL(0, buffer[i]);
    }
}

TEST(MemoryOperations, BufferWrite)
{
    // Test writing to buffer
    buffer[0] = 0x12;
    buffer[1] = 0x34;
    LONGS_EQUAL(0x12, buffer[0]);
    LONGS_EQUAL(0x34, buffer[1]);
}
