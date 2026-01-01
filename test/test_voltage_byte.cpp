/**
 * @file test_voltage_byte.cpp
 * @brief Unit tests for VoltageByte class
 */

#include "CppUTest/TestHarness.h"
#include "VoltageByte.h"

TEST_GROUP(VoltageByte_Construction)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(VoltageByte_Construction, DefaultConstructor)
{
    VoltageByte v;
    LONGS_EQUAL(0, v.get());
}

TEST(VoltageByte_Construction, ExplicitConstructor)
{
    VoltageByte v(100);
    LONGS_EQUAL(100, v.get());
}

TEST(VoltageByte_Construction, MinValue)
{
    VoltageByte v(VoltageByte::MIN_VALUE);
    LONGS_EQUAL(0, v.get());
}

TEST(VoltageByte_Construction, MaxValue)
{
    VoltageByte v(VoltageByte::MAX_VALUE);
    LONGS_EQUAL(255, v.get());
}

TEST_GROUP(VoltageByte_Conversion)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(VoltageByte_Conversion, FromMinimumVoltage)
{
    // 2.75V -> (2.75 * 100) - 210 = 275 - 210 = 65
    VoltageByte v = VoltageByte::fromVoltage(2.75f);
    LONGS_EQUAL(65, v.get());
}

TEST(VoltageByte_Conversion, FromNominalVoltage)
{
    // 3.7V -> (3.7 * 100) - 210 = 370 - 210 = 160
    VoltageByte v = VoltageByte::fromVoltage(3.7f);
    LONGS_EQUAL(160, v.get());
}

TEST(VoltageByte_Conversion, FromMaximumVoltage)
{
    // 4.2V -> (4.2 * 100) - 210 = 420 - 210 = 210
    VoltageByte v = VoltageByte::fromVoltage(4.2f);
    // Accept 209 or 210 due to floating point precision
    CHECK(v.get() >= 209 && v.get() <= 210);
}

TEST(VoltageByte_Conversion, ToVoltageFromByte65)
{
    // byte 65 -> (65 + 210) / 100 = 2.75V
    VoltageByte v(65);
    DOUBLES_EQUAL(2.75f, v.toVoltage(), 0.01f);
}

TEST(VoltageByte_Conversion, ToVoltageFromByte160)
{
    // byte 160 -> (160 + 210) / 100 = 3.70V
    VoltageByte v(160);
    DOUBLES_EQUAL(3.70f, v.toVoltage(), 0.01f);
}

TEST(VoltageByte_Conversion, ToVoltageFromByte210)
{
    // byte 210 -> (210 + 210) / 100 = 4.20V
    VoltageByte v(210);
    DOUBLES_EQUAL(4.20f, v.toVoltage(), 0.01f);
}

TEST(VoltageByte_Conversion, RoundTrip)
{
    // Test that fromVoltage -> toVoltage round trips correctly
    float original = 3.6f;
    VoltageByte v = VoltageByte::fromVoltage(original);
    float result = v.toVoltage();
    DOUBLES_EQUAL(original, result, 0.01f);
}

TEST(VoltageByte_Conversion, ClampLowVoltage)
{
    // Voltage below minimum should clamp to 0
    VoltageByte v = VoltageByte::fromVoltage(1.0f);
    LONGS_EQUAL(0, v.get());
}

TEST(VoltageByte_Conversion, ClampHighVoltage)
{
    // Voltage above maximum should clamp to 255
    VoltageByte v = VoltageByte::fromVoltage(10.0f);
    LONGS_EQUAL(255, v.get());
}

TEST(VoltageByte_Conversion, ExplicitCastToUint8)
{
    VoltageByte v(123);
    uint8_t raw = static_cast<uint8_t>(v);
    LONGS_EQUAL(123, raw);
}

TEST_GROUP(VoltageByte_Comparison)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(VoltageByte_Comparison, Equality)
{
    VoltageByte v1(100);
    VoltageByte v2(100);
    VoltageByte v3(101);
    
    CHECK(v1 == v2);
    CHECK_FALSE(v1 == v3);
}

TEST(VoltageByte_Comparison, Inequality)
{
    VoltageByte v1(100);
    VoltageByte v2(101);
    
    CHECK(v1 != v2);
    CHECK_FALSE(v1 != v1);
}

TEST(VoltageByte_Comparison, LessThan)
{
    VoltageByte v1(100);
    VoltageByte v2(101);
    
    CHECK(v1 < v2);
    CHECK_FALSE(v2 < v1);
    CHECK_FALSE(v1 < v1);
}

TEST(VoltageByte_Comparison, GreaterThan)
{
    VoltageByte v1(100);
    VoltageByte v2(101);
    
    CHECK(v2 > v1);
    CHECK_FALSE(v1 > v2);
    CHECK_FALSE(v1 > v1);
}

TEST(VoltageByte_Comparison, LessThanOrEqual)
{
    VoltageByte v1(100);
    VoltageByte v2(101);
    
    CHECK(v1 <= v2);
    CHECK(v1 <= v1);
    CHECK_FALSE(v2 <= v1);
}

TEST(VoltageByte_Comparison, GreaterThanOrEqual)
{
    VoltageByte v1(100);
    VoltageByte v2(101);
    
    CHECK(v2 >= v1);
    CHECK(v1 >= v1);
    CHECK_FALSE(v1 >= v2);
}




TEST_GROUP(VoltageByte_UsageScenarios)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(VoltageByte_UsageScenarios, CANBufferWrite)
{
    // Simulate writing voltage to CAN buffer
    uint8_t can_buffer[8] = {0};
    VoltageByte voltage = VoltageByte::fromVoltage(3.7f);
    
    // Must use explicit cast to write to buffer
    can_buffer[0] = static_cast<uint8_t>(voltage);
    
    LONGS_EQUAL(160, can_buffer[0]);
}

TEST(VoltageByte_UsageScenarios, CANBufferRead)
{
    // Simulate reading voltage from CAN buffer
    uint8_t can_buffer[8] = {160, 0, 0, 0, 0, 0, 0, 0};
    
    // Must use explicit constructor to create from raw byte
    VoltageByte voltage(can_buffer[0]);
    
    DOUBLES_EQUAL(3.7f, voltage.toVoltage(), 0.01f);
}

TEST(VoltageByte_UsageScenarios, CompareVoltages)
{
    // Find minimum and maximum cell voltage
    VoltageByte cell1 = VoltageByte::fromVoltage(3.65f);
    VoltageByte cell2 = VoltageByte::fromVoltage(3.70f);
    VoltageByte cell3 = VoltageByte::fromVoltage(3.68f);
    
    // Can use comparison operators directly
    CHECK(cell1 < cell2);
    CHECK(cell2 > cell3);
    CHECK(cell3 > cell1);
}

TEST_GROUP(VoltageByte_Constants)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(VoltageByte_Constants, GetMaxVoltage)
{
    VoltageByte maxVolt = VoltageByte::getMaxVoltage();
    DOUBLES_EQUAL(4.20f, maxVolt.toVoltage(), 0.01f);
}

TEST(VoltageByte_Constants, GetMinVoltage)
{
    VoltageByte minVolt = VoltageByte::getMinVoltage();
    DOUBLES_EQUAL(2.75f, minVolt.toVoltage(), 0.01f);
}