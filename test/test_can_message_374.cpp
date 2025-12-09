/**
 * @file test_can_message_374.cpp
 * @brief Unit tests for CanMessage374 class
 */

#include "CppUTest/TestHarness.h"
#include "CanMessage374.h"
#include <string.h>

TEST_GROUP(CanMessage374_Construction)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = CanMessage374::MESSAGE_ID;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_Construction, ConstructFromFrame)
{
    CanMessage374 msg(&frame);
    CHECK(msg.getFrame() != nullptr);
    LONGS_EQUAL(0x374, msg.getFrame()->ID);
}

TEST(CanMessage374_Construction, ConstantsAreCorrect)
{
    LONGS_EQUAL(0x374, CanMessage374::MESSAGE_ID);
    LONGS_EQUAL(100, CanMessage374::RECURRENCE_MS);
}

TEST_GROUP(CanMessage374_SoCAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_SoCAccess, GetSoC1_ZeroPercent)
{
    frame.data[0] = 10;  // (10 - 10) / 2 = 0%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC1();
    DOUBLES_EQUAL(0.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, GetSoC1_FiftyPercent)
{
    frame.data[0] = 110;  // (110 - 10) / 2 = 50%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC1();
    DOUBLES_EQUAL(50.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, GetSoC1_HundredPercent)
{
    frame.data[0] = 210;  // (210 - 10) / 2 = 100%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC1();
    DOUBLES_EQUAL(100.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, GetSoC2_ZeroPercent)
{
    frame.data[1] = 10;  // (10 - 10) / 2 = 0%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC2();
    DOUBLES_EQUAL(0.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, GetSoC2_SeventyFivePercent)
{
    frame.data[1] = 160;  // (160 - 10) / 2 = 75%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC2();
    DOUBLES_EQUAL(75.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, GetSoC2_HundredPercent)
{
    frame.data[1] = 210;  // (210 - 10) / 2 = 100%
    CanMessage374 msg(&frame);
    
    float soc = msg.getSoC2();
    DOUBLES_EQUAL(100.0f, soc, 0.01f);
}

TEST(CanMessage374_SoCAccess, SetSoC1_ZeroPercent)
{
    CanMessage374 msg(&frame);
    msg.setSoC1(0.0f);
    
    LONGS_EQUAL(10, frame.data[0]);
    DOUBLES_EQUAL(0.0f, msg.getSoC1(), 0.5f);
}

TEST(CanMessage374_SoCAccess, SetSoC1_FiftyPercent)
{
    CanMessage374 msg(&frame);
    msg.setSoC1(50.0f);
    
    LONGS_EQUAL(110, frame.data[0]);
    DOUBLES_EQUAL(50.0f, msg.getSoC1(), 0.5f);
}

TEST(CanMessage374_SoCAccess, SetSoC1_HundredPercent)
{
    CanMessage374 msg(&frame);
    msg.setSoC1(100.0f);
    
    LONGS_EQUAL(210, frame.data[0]);
    DOUBLES_EQUAL(100.0f, msg.getSoC1(), 0.5f);
}

TEST(CanMessage374_SoCAccess, SetSoC2_TwentyFivePercent)
{
    CanMessage374 msg(&frame);
    msg.setSoC2(25.0f);
    
    LONGS_EQUAL(60, frame.data[1]);
    DOUBLES_EQUAL(25.0f, msg.getSoC2(), 0.5f);
}

TEST(CanMessage374_SoCAccess, SetSoC_ClampNegative)
{
    CanMessage374 msg(&frame);
    msg.setSoC1(-10.0f);
    
    LONGS_EQUAL(10, frame.data[0]);  // Should clamp to 0%
    DOUBLES_EQUAL(0.0f, msg.getSoC1(), 0.5f);
}

TEST(CanMessage374_SoCAccess, SetSoC_ClampAboveHundred)
{
    CanMessage374 msg(&frame);
    msg.setSoC2(150.0f);
    
    LONGS_EQUAL(210, frame.data[1]);  // Should clamp to 100%
    DOUBLES_EQUAL(100.0f, msg.getSoC2(), 0.5f);
}

TEST(CanMessage374_SoCAccess, RoundTripSoC1)
{
    CanMessage374 msg(&frame);
    msg.setSoC1(33.5f);
    
    float retrieved = msg.getSoC1();
    DOUBLES_EQUAL(33.5f, retrieved, 0.5f);
}

TEST_GROUP(CanMessage374_TemperatureAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_TemperatureAccess, GetMaxTemp_ZeroDegrees)
{
    frame.data[4] = 50;  // 50 - 50 = 0°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMaxTemperature();
    DOUBLES_EQUAL(0.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, GetMaxTemp_TwentyFiveDegrees)
{
    frame.data[4] = 75;  // 75 - 50 = 25°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMaxTemperature();
    DOUBLES_EQUAL(25.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, GetMaxTemp_NegativeTen)
{
    frame.data[4] = 40;  // 40 - 50 = -10°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMaxTemperature();
    DOUBLES_EQUAL(-10.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, GetMinTemp_ZeroDegrees)
{
    frame.data[5] = 50;  // 50 - 50 = 0°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMinTemperature();
    DOUBLES_EQUAL(0.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, GetMinTemp_FiveDegrees)
{
    frame.data[5] = 55;  // 55 - 50 = 5°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMinTemperature();
    DOUBLES_EQUAL(5.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, GetMinTemp_MinusFive)
{
    frame.data[5] = 45;  // 45 - 50 = -5°C
    CanMessage374 msg(&frame);
    
    float temp = msg.getCellMinTemperature();
    DOUBLES_EQUAL(-5.0f, temp, 0.01f);
}

TEST(CanMessage374_TemperatureAccess, SetMaxTemp_TwentyDegrees)
{
    CanMessage374 msg(&frame);
    msg.setCellMaxTemperature(20.0f);
    
    LONGS_EQUAL(70, frame.data[4]);  // 20 + 50 = 70
    DOUBLES_EQUAL(20.0f, msg.getCellMaxTemperature(), 0.5f);
}

TEST(CanMessage374_TemperatureAccess, SetMinTemp_FiveDegrees)
{
    CanMessage374 msg(&frame);
    msg.setCellMinTemperature(5.0f);
    
    LONGS_EQUAL(55, frame.data[5]);  // 5 + 50 = 55
    DOUBLES_EQUAL(5.0f, msg.getCellMinTemperature(), 0.5f);
}

TEST(CanMessage374_TemperatureAccess, SetTemp_NegativeValue)
{
    CanMessage374 msg(&frame);
    msg.setCellMaxTemperature(-10.0f);
    
    LONGS_EQUAL(40, frame.data[4]);  // -10 + 50 = 40
    DOUBLES_EQUAL(-10.0f, msg.getCellMaxTemperature(), 0.5f);
}

TEST(CanMessage374_TemperatureAccess, SetTemp_ClampLow)
{
    CanMessage374 msg(&frame);
    msg.setCellMinTemperature(-60.0f);  // Would be -10, clamped to 0
    
    LONGS_EQUAL(0, frame.data[5]);
    DOUBLES_EQUAL(-50.0f, msg.getCellMinTemperature(), 0.5f);
}

TEST(CanMessage374_TemperatureAccess, SetTemp_ClampHigh)
{
    CanMessage374 msg(&frame);
    msg.setCellMaxTemperature(250.0f);  // Would be 300, clamped to 255
    
    LONGS_EQUAL(255, frame.data[4]);
    DOUBLES_EQUAL(205.0f, msg.getCellMaxTemperature(), 0.5f);
}

TEST(CanMessage374_TemperatureAccess, RoundTripTemperature)
{
    CanMessage374 msg(&frame);
    msg.setCellMaxTemperature(37.5f);
    
    float retrieved = msg.getCellMaxTemperature();
    DOUBLES_EQUAL(37.5f, retrieved, 0.5f);
}

TEST_GROUP(CanMessage374_CapacityAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_CapacityAccess, GetCapacity_ZeroAh)
{
    frame.data[6] = 0;  // 0 / 2 = 0 Ah
    CanMessage374 msg(&frame);
    
    float capacity = msg.getBatteryCapacity();
    DOUBLES_EQUAL(0.0f, capacity, 0.01f);
}

TEST(CanMessage374_CapacityAccess, GetCapacity_FiftyAh)
{
    frame.data[6] = 100;  // 100 / 2 = 50 Ah
    CanMessage374 msg(&frame);
    
    float capacity = msg.getBatteryCapacity();
    DOUBLES_EQUAL(50.0f, capacity, 0.01f);
}

TEST(CanMessage374_CapacityAccess, GetCapacity_NinetyAh)
{
    frame.data[6] = 180;  // 180 / 2 = 90 Ah
    CanMessage374 msg(&frame);
    
    float capacity = msg.getBatteryCapacity();
    DOUBLES_EQUAL(90.0f, capacity, 0.01f);
}

TEST(CanMessage374_CapacityAccess, GetCapacity_MaxValue)
{
    frame.data[6] = 255;  // 255 / 2 = 127.5 Ah
    CanMessage374 msg(&frame);
    
    float capacity = msg.getBatteryCapacity();
    DOUBLES_EQUAL(127.5f, capacity, 0.01f);
}

TEST(CanMessage374_CapacityAccess, SetCapacity_FiftyAh)
{
    CanMessage374 msg(&frame);
    msg.setBatteryCapacity(50.0f);
    
    LONGS_EQUAL(100, frame.data[6]);  // 50 * 2 = 100
    DOUBLES_EQUAL(50.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_CapacityAccess, SetCapacity_NinetyAh)
{
    CanMessage374 msg(&frame);
    msg.setBatteryCapacity(90.0f);
    
    LONGS_EQUAL(180, frame.data[6]);  // 90 * 2 = 180
    DOUBLES_EQUAL(90.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_CapacityAccess, SetCapacity_WithDecimal)
{
    CanMessage374 msg(&frame);
    msg.setBatteryCapacity(45.5f);
    
    LONGS_EQUAL(91, frame.data[6]);  // 45.5 * 2 = 91
    DOUBLES_EQUAL(45.5f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_CapacityAccess, SetCapacity_ClampHigh)
{
    CanMessage374 msg(&frame);
    msg.setBatteryCapacity(200.0f);  // Would be 400, clamped to 255
    
    LONGS_EQUAL(255, frame.data[6]);
    DOUBLES_EQUAL(127.5f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_CapacityAccess, RoundTripCapacity)
{
    CanMessage374 msg(&frame);
    msg.setBatteryCapacity(93.0f);
    
    float retrieved = msg.getBatteryCapacity();
    DOUBLES_EQUAL(93.0f, retrieved, 0.5f);
}

TEST_GROUP(CanMessage374_RawDataAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
        
        // Fill with test pattern
        for (int i = 0; i < 8; i++) {
            frame.data[i] = i * 20;
        }
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_RawDataAccess, GetRawDataAllBytes)
{
    CanMessage374 msg(&frame);
    
    for (uint8_t i = 0; i < 8; i++) {
        LONGS_EQUAL(i * 20, msg.getRawData(i));
    }
}

TEST(CanMessage374_RawDataAccess, GetRawDataOutOfBounds)
{
    CanMessage374 msg(&frame);
    
    // Should return 0 for out of bounds
    LONGS_EQUAL(0, msg.getRawData(8));
    LONGS_EQUAL(0, msg.getRawData(255));
}

TEST_GROUP(CanMessage374_RealWorldScenarios)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_RealWorldScenarios, FullyChargedBattery)
{
    // Fully charged battery scenario:
    // SoC1: 100% (byte 210)
    // SoC2: 100% (byte 210)
    // Max temp: 35°C (byte 85)
    // Min temp: 30°C (byte 80)
    // Capacity: 93 Ah (byte 186)
    
    frame.data[0] = 210;
    frame.data[1] = 210;
    frame.data[4] = 85;
    frame.data[5] = 80;
    frame.data[6] = 186;
    
    CanMessage374 msg(&frame);
    
    DOUBLES_EQUAL(100.0f, msg.getSoC1(), 0.5f);
    DOUBLES_EQUAL(100.0f, msg.getSoC2(), 0.5f);
    DOUBLES_EQUAL(35.0f, msg.getCellMaxTemperature(), 0.5f);
    DOUBLES_EQUAL(30.0f, msg.getCellMinTemperature(), 0.5f);
    DOUBLES_EQUAL(93.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_RealWorldScenarios, HalfChargedBattery)
{
    // Half charged battery:
    // SoC1: 48% (byte 106)
    // SoC2: 52% (byte 114)
    // Max temp: 22°C (byte 72)
    // Min temp: 20°C (byte 70)
    // Capacity: 90 Ah (byte 180)
    
    frame.data[0] = 106;
    frame.data[1] = 114;
    frame.data[4] = 72;
    frame.data[5] = 70;
    frame.data[6] = 180;
    
    CanMessage374 msg(&frame);
    
    DOUBLES_EQUAL(48.0f, msg.getSoC1(), 0.5f);
    DOUBLES_EQUAL(52.0f, msg.getSoC2(), 0.5f);
    DOUBLES_EQUAL(22.0f, msg.getCellMaxTemperature(), 0.5f);
    DOUBLES_EQUAL(20.0f, msg.getCellMinTemperature(), 0.5f);
    DOUBLES_EQUAL(90.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_RealWorldScenarios, LowBatteryColdWeather)
{
    // Low battery in cold weather:
    // SoC1: 15% (byte 40)
    // SoC2: 12% (byte 34)
    // Max temp: 2°C (byte 52)
    // Min temp: -3°C (byte 47)
    // Capacity: 88 Ah (byte 176)
    
    frame.data[0] = 40;
    frame.data[1] = 34;
    frame.data[4] = 52;
    frame.data[5] = 47;
    frame.data[6] = 176;
    
    CanMessage374 msg(&frame);
    
    DOUBLES_EQUAL(15.0f, msg.getSoC1(), 0.5f);
    DOUBLES_EQUAL(12.0f, msg.getSoC2(), 0.5f);
    DOUBLES_EQUAL(2.0f, msg.getCellMaxTemperature(), 0.5f);
    DOUBLES_EQUAL(-3.0f, msg.getCellMinTemperature(), 0.5f);
    DOUBLES_EQUAL(88.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_RealWorldScenarios, BuildCompleteMessage)
{
    // Build a complete message from scratch
    CanMessage374 msg(&frame);
    
    msg.setSoC1(75.5f);
    msg.setSoC2(73.0f);
    msg.setCellMaxTemperature(28.0f);
    msg.setCellMinTemperature(25.0f);
    msg.setBatteryCapacity(92.0f);
    
    // Verify all values
    DOUBLES_EQUAL(75.5f, msg.getSoC1(), 0.5f);
    DOUBLES_EQUAL(73.0f, msg.getSoC2(), 0.5f);
    DOUBLES_EQUAL(28.0f, msg.getCellMaxTemperature(), 0.5f);
    DOUBLES_EQUAL(25.0f, msg.getCellMinTemperature(), 0.5f);
    DOUBLES_EQUAL(92.0f, msg.getBatteryCapacity(), 0.5f);
}

TEST(CanMessage374_RealWorldScenarios, TemperatureDelta)
{
    // Check temperature imbalance
    frame.data[4] = 80;  // 30°C max
    frame.data[5] = 70;  // 20°C min
    
    CanMessage374 msg(&frame);
    
    float maxTemp = msg.getCellMaxTemperature();
    float minTemp = msg.getCellMinTemperature();
    float delta = maxTemp - minTemp;
    
    DOUBLES_EQUAL(10.0f, delta, 0.5f);
}

TEST(CanMessage374_RealWorldScenarios, SoCDiscrepancy)
{
    // Scenario where voltage-based SoC differs from coulomb counting
    frame.data[0] = 120;  // SoC1 = 55%
    frame.data[1] = 110;  // SoC2 = 50%
    
    CanMessage374 msg(&frame);
    
    float soc1 = msg.getSoC1();
    float soc2 = msg.getSoC2();
    float discrepancy = soc1 - soc2;
    
    DOUBLES_EQUAL(5.0f, discrepancy, 0.5f);
}

TEST_GROUP(CanMessage374_FramePointerAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x374;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage374_FramePointerAccess, GetFramePointer)
{
    CanMessage374 msg(&frame);
    
    CAN_FRAME* framePtr = msg.getFrame();
    CHECK(framePtr == &frame);
    LONGS_EQUAL(0x374, framePtr->ID);
}

TEST(CanMessage374_FramePointerAccess, ConstGetFramePointer)
{
    const CanMessage374 msg(&frame);
    
    const CAN_FRAME* framePtr = msg.getFrame();
    CHECK(framePtr == &frame);
    LONGS_EQUAL(0x374, framePtr->ID);
}

TEST(CanMessage374_FramePointerAccess, ModifyThroughFramePointer)
{
    CanMessage374 msg(&frame);
    
    CAN_FRAME* framePtr = msg.getFrame();
    framePtr->data[0] = 110;  // 50% SoC
    
    float soc = msg.getSoC1();
    DOUBLES_EQUAL(50.0f, soc, 0.5f);
}
