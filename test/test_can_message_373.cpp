/**
 * @file test_can_message_373.cpp
 * @brief Unit tests for CanMessage373 class
 */

#include "CppUTest/TestHarness.h"
#include "CanMessage373.h"
#include <string.h>

TEST_GROUP(CanMessage373_Construction)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = CanMessage373::MESSAGE_ID;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};


TEST_GROUP(CanMessage373_VoltageAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage373_VoltageAccess, GetCellMaxVoltage)
{
    frame.data[0] = 160;  // 3.70V
    CanMessage373 msg(&frame);
    
    VoltageByte maxVoltage = msg.getCellMaxVoltage();
    LONGS_EQUAL(160, maxVoltage.get());
    DOUBLES_EQUAL(3.70f, maxVoltage.toVoltage(), 0.01f);
}

TEST(CanMessage373_VoltageAccess, GetCellMinVoltage)
{
    frame.data[1] = 155;  // 3.65V
    CanMessage373 msg(&frame);
    
    VoltageByte minVoltage = msg.getCellMinVoltage();
    LONGS_EQUAL(155, minVoltage.get());
    DOUBLES_EQUAL(3.65f, minVoltage.toVoltage(), 0.01f);
}


TEST_GROUP(CanMessage373_CurrentAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage373_CurrentAccess, ZeroCurrent)
{
    // Zero current: 32700 = 0x7FBC
    frame.data[2] = 0x7F;  // High byte
    frame.data[3] = 0xBC;  // Low byte
    
    CanMessage373 msg(&frame);
    float current = msg.getPackCurrent();
    
    DOUBLES_EQUAL(0.0f, current, 0.01f);
}

TEST(CanMessage373_CurrentAccess, PositiveCurrent_Charging)
{
    // +50A charging: 32700 + 5000 = 37700 = 0x9344
    frame.data[2] = 0x93;
    frame.data[3] = 0x44;
    
    CanMessage373 msg(&frame);
    float current = msg.getPackCurrent();
    
    DOUBLES_EQUAL(50.0f, current, 0.1f);
}

TEST(CanMessage373_CurrentAccess, NegativeCurrent_Discharging)
{
    // -100A discharging: 32700 - 10000 = 22700 = 0x58AC
    frame.data[2] = 0x58;
    frame.data[3] = 0xAC;
    
    CanMessage373 msg(&frame);
    float current = msg.getPackCurrent();
    
    DOUBLES_EQUAL(-100.0f, current, 0.1f);
}

TEST(CanMessage373_CurrentAccess, SmallPositiveCurrent)
{
    // +2.66A: 32700 + 266 = 32966 = 0x80C6
    frame.data[2] = 0x80;
    frame.data[3] = 0xC6;
    
    CanMessage373 msg(&frame);
    float current = msg.getPackCurrent();
    
    DOUBLES_EQUAL(2.66f, current, 0.1f);
}

TEST(CanMessage373_CurrentAccess, SmallNegativeCurrent)
{
    // -1.5A: 32700 - 150 = 32550 = 0x7F26
    frame.data[2] = 0x7F;
    frame.data[3] = 0x26;
    
    CanMessage373 msg(&frame);
    float current = msg.getPackCurrent();
    
    DOUBLES_EQUAL(-1.5f, current, 0.1f);
}

TEST_GROUP(CanMessage373_PackVoltageAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage373_PackVoltageAccess, NominalPackVoltage)
{
    // 370.0V: 3700 = 0x0E74
    frame.data[4] = 0x0E;
    frame.data[5] = 0x74;
    
    CanMessage373 msg(&frame);
    float voltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(370.0f, voltage, 0.1f);
}

TEST(CanMessage373_PackVoltageAccess, LowPackVoltage)
{
    // 275.0V: 2750 = 0x0ABE
    frame.data[4] = 0x0A;
    frame.data[5] = 0xBE;
    
    CanMessage373 msg(&frame);
    float voltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(275.0f, voltage, 0.1f);
}

TEST(CanMessage373_PackVoltageAccess, HighPackVoltage)
{
    // 420.0V: 4200 = 0x1068
    frame.data[4] = 0x10;
    frame.data[5] = 0x68;
    
    CanMessage373 msg(&frame);
    float voltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(420.0f, voltage, 0.1f);
}

TEST(CanMessage373_PackVoltageAccess, ZeroVoltage)
{
    frame.data[4] = 0x00;
    frame.data[5] = 0x00;
    
    CanMessage373 msg(&frame);
    float voltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(0.0f, voltage, 0.01f);
}

TEST_GROUP(CanMessage373_RawDataAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
        
        // Fill with test pattern
        for (int i = 0; i < 8; i++) {
            frame.data[i] = i * 10;
        }
    }
    
    void teardown() {
    }
};


TEST_GROUP(CanMessage373_RealWorldScenarios)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

TEST(CanMessage373_RealWorldScenarios, TypicalDrivingConditions)
{
    // Typical driving scenario:
    // Max cell: 3.72V (byte 162)
    // Min cell: 3.68V (byte 158)
    // Current: -75A discharging (32700 - 7500 = 25200)
    // Pack voltage: 360V (3600)
    
    frame.data[0] = 162;  // Max cell
    frame.data[1] = 158;  // Min cell
    frame.data[2] = 0x62;  // Current high byte (25200 = 0x6270)
    frame.data[3] = 0x70;  // Current low byte
    frame.data[4] = 0x0E;  // Pack voltage high (3600 = 0x0E10)
    frame.data[5] = 0x10;  // Pack voltage low
    
    CanMessage373 msg(&frame);
    
    VoltageByte maxVoltage = msg.getCellMaxVoltage();
    VoltageByte minVoltage = msg.getCellMinVoltage();
    float current = msg.getPackCurrent();
    float packVoltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(3.72f, maxVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(3.68f, minVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(-75.0f, current, 0.5f);
    DOUBLES_EQUAL(360.0f, packVoltage, 0.5f);
}

TEST(CanMessage373_RealWorldScenarios, ChargingScenario)
{
    // Charging scenario:
    // Max cell: 4.15V (byte 205)
    // Min cell: 4.10V (byte 200)
    // Current: +30A charging (32700 + 3000 = 35700)
    // Pack voltage: 410V (4100)
    
    frame.data[0] = 205;
    frame.data[1] = 200;
    frame.data[2] = 0x8B;  // 35700 = 0x8B8C
    frame.data[3] = 0x8C;
    frame.data[4] = 0x10;  // 4100 = 0x1004
    frame.data[5] = 0x04;
    
    CanMessage373 msg(&frame);
    
    VoltageByte maxVoltage = msg.getCellMaxVoltage();
    VoltageByte minVoltage = msg.getCellMinVoltage();
    float current = msg.getPackCurrent();
    float packVoltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(4.15f, maxVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(4.10f, minVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(30.0f, current, 0.5f);
    DOUBLES_EQUAL(410.0f, packVoltage, 0.5f);
}

TEST(CanMessage373_RealWorldScenarios, RestingBattery)
{
    // Battery at rest (near zero current):
    // Max cell: 3.70V (byte 160)
    // Min cell: 3.68V (byte 158)
    // Current: +0.5A (32700 + 50 = 32750)
    // Pack voltage: 368V (3680)
    
    frame.data[0] = 160;
    frame.data[1] = 158;
    frame.data[2] = 0x7F;  // 32750 = 0x7FEE
    frame.data[3] = 0xEE;
    frame.data[4] = 0x0E;  // 3680 = 0x0E60
    frame.data[5] = 0x60;
    
    CanMessage373 msg(&frame);
    
    VoltageByte maxVoltage = msg.getCellMaxVoltage();
    VoltageByte minVoltage = msg.getCellMinVoltage();
    float current = msg.getPackCurrent();
    float packVoltage = msg.getPackVoltage();
    
    DOUBLES_EQUAL(3.70f, maxVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(3.68f, minVoltage.toVoltage(), 0.01f);
    DOUBLES_EQUAL(0.5f, current, 0.1f);
    DOUBLES_EQUAL(368.0f, packVoltage, 0.5f);
}

TEST(CanMessage373_RealWorldScenarios, CalculateCellVoltageDelta)
{
    // Check cell voltage imbalance
    frame.data[0] = 165;  // 3.75V max
    frame.data[1] = 155;  // 3.65V min
    
    CanMessage373 msg(&frame);
    
    VoltageByte maxVoltage = msg.getCellMaxVoltage();
    VoltageByte minVoltage = msg.getCellMinVoltage();
    
    float delta = maxVoltage.toVoltage() - minVoltage.toVoltage();
    
    DOUBLES_EQUAL(0.10f, delta, 0.01f);  // 100mV imbalance
}

TEST_GROUP(CanMessage373_FramePointerAccess)
{
    CAN_FRAME frame;
    
    void setup() {
        memset(&frame, 0, sizeof(CAN_FRAME));
        frame.ID = 0x373;
        frame.dlc = 8;
    }
    
    void teardown() {
    }
};

