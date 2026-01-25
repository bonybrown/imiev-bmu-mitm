#include "CppUTest/TestHarness.h"
#include "CanMessage374Override.h"
#include "CanMessage374.h"
#include "can_types.h"

TEST_GROUP(CanMessage374Override)
{
    CanMessage374Override* tempOverride;
    CAN_FRAME frame;

    void setup()
    {
        tempOverride = new CanMessage374Override();
        
        // Initialize a valid CAN frame for message 0x374
        frame.ID = 0x374;
        frame.dlc = 8;
        frame.ide = 0;
        frame.rtr = 0;
        frame.rx_channel = 0;
        // Set initial temperature values: 25°C min, 30°C max
        frame.data[4] = TemperatureValue(30.0f).toCanByte(); // 80
        frame.data[5] = TemperatureValue(25.0f).toCanByte(); // 75
    }

    void teardown()
    {
        delete tempOverride;
    }
};

TEST(CanMessage374Override, InitiallyNotActive)
{
    CHECK_FALSE(tempOverride->isOverrideActive());
    CHECK_EQUAL(0, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, SetOverrideMakesActive)
{
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 10);
    
    CHECK_TRUE(tempOverride->isOverrideActive());
    CHECK_EQUAL(10, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, ApplyOverrideChangesTemperatures)
{
    CanMessage374 msg(&frame);
    
    // Verify initial values
    DOUBLES_EQUAL(25.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(30.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
    
    // Set and apply override
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 10);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_TRUE(applied);
    DOUBLES_EQUAL(20.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(35.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, StoresOriginalValues)
{
    CanMessage374 msg(&frame);
    
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 10);
    tempOverride->applyOverride(msg);
    
    DOUBLES_EQUAL(25.0f, tempOverride->getOriginalMinTemp().celsius(), 0.1f);
    DOUBLES_EQUAL(30.0f, tempOverride->getOriginalMaxTemp().celsius(), 0.1f);
}

TEST(CanMessage374Override, DecrementsFrameCounter)
{
    CanMessage374 msg(&frame);
    
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 3);
    CHECK_EQUAL(3, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(2, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(1, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(0, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, ExpiresAfterFrameCount)
{
    CanMessage374 msg(&frame);
    
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 2);
    CHECK_TRUE(tempOverride->isOverrideActive());
    
    tempOverride->applyOverride(msg);
    CHECK_TRUE(tempOverride->isOverrideActive());
    
    tempOverride->applyOverride(msg);
    CHECK_FALSE(tempOverride->isOverrideActive());
    
    // Applying again should return false
    bool applied = tempOverride->applyOverride(msg);
    CHECK_FALSE(applied);
}

TEST(CanMessage374Override, DoesNotApplyWhenInactive)
{
    CanMessage374 msg(&frame);
    
    // Get initial values
    float originalMin = msg.getCellMinTemperature().celsius();
    float originalMax = msg.getCellMaxTemperature().celsius();
    
    // Try to apply without setting override
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_FALSE(applied);
    DOUBLES_EQUAL(originalMin, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(originalMax, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, ClearOverrideDeactivates)
{
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 10);
    CHECK_TRUE(tempOverride->isOverrideActive());
    
    tempOverride->clearOverride();
    CHECK_FALSE(tempOverride->isOverrideActive());
    CHECK_EQUAL(0, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, ClearOverridePreventsApplication)
{
    CanMessage374 msg(&frame);
    
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 10);
    tempOverride->clearOverride();
    
    bool applied = tempOverride->applyOverride(msg);
    CHECK_FALSE(applied);
    
    // Values should be unchanged
    DOUBLES_EQUAL(25.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(30.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, ZeroFrameCountIsNotActive)
{
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(35.0f), 0);
    CHECK_FALSE(tempOverride->isOverrideActive());
    CHECK_EQUAL(0, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, MultipleOverridesInSequence)
{
    CanMessage374 msg(&frame);
    
    // First override
    tempOverride->setOverride(TemperatureValue(15.0f), TemperatureValue(40.0f), 1);
    tempOverride->applyOverride(msg);
    DOUBLES_EQUAL(15.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(40.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
    
    // Reset frame values
    msg.setCellMinTemperature(TemperatureValue(25.0f));
    msg.setCellMaxTemperature(TemperatureValue(30.0f));
    
    // Second override
    tempOverride->setOverride(TemperatureValue(10.0f), TemperatureValue(45.0f), 2);
    tempOverride->applyOverride(msg);
    DOUBLES_EQUAL(10.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(45.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
    CHECK_EQUAL(1, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, RejectsUnrealisticCoolingAbove25C)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 30°C (above 25°C)
    msg.setCellMinTemperature(TemperatureValue(28.0f));
    msg.setCellMaxTemperature(TemperatureValue(30.0f));
    
    // Try to override with temp below 25°C
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(24.0f), 5);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_FALSE(applied);
    // Temperature should remain unchanged
    DOUBLES_EQUAL(28.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(30.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
    // Frame counter should still decrement
    CHECK_EQUAL(4, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, RejectsUnrealisticHeatingBelow10C)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 8°C (below 10°C)
    msg.setCellMinTemperature(TemperatureValue(5.0f));
    msg.setCellMaxTemperature(TemperatureValue(8.0f));
    
    // Try to override with temp > (8 + 10) = 18°C
    tempOverride->setOverride(TemperatureValue(15.0f), TemperatureValue(20.0f), 5);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_FALSE(applied);
    // Temperature should remain unchanged
    DOUBLES_EQUAL(5.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(8.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
    // Frame counter should still decrement
    CHECK_EQUAL(4, tempOverride->getRemainingFrames());
}

TEST(CanMessage374Override, AllowsHeatingUpTo10CAboveColdTemp)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 8°C
    msg.setCellMinTemperature(TemperatureValue(5.0f));
    msg.setCellMaxTemperature(TemperatureValue(8.0f));
    
    // Override with temp = 8 + 10 = 18°C (exactly at limit)
    tempOverride->setOverride(TemperatureValue(12.0f), TemperatureValue(18.0f), 1);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_TRUE(applied);
    DOUBLES_EQUAL(12.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(18.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, AllowsCoolingToExactly25C)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 30°C
    msg.setCellMinTemperature(TemperatureValue(28.0f));
    msg.setCellMaxTemperature(TemperatureValue(30.0f));
    
    // Override with temp = exactly 25°C (at limit)
    tempOverride->setOverride(TemperatureValue(23.0f), TemperatureValue(25.0f), 1);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_TRUE(applied);
    DOUBLES_EQUAL(23.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(25.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, AllowsFullRangeWhenTempBetween10And25)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 20°C (between 10 and 25)
    msg.setCellMinTemperature(TemperatureValue(18.0f));
    msg.setCellMaxTemperature(TemperatureValue(20.0f));
    
    // Should allow override from -50 to +50°C
    tempOverride->setOverride(TemperatureValue(-50.0f), TemperatureValue(50.0f), 1);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_TRUE(applied);
    DOUBLES_EQUAL(-50.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(50.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, AllowsExtremeNegativeWhenTempBetween10And25)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 15°C
    msg.setCellMinTemperature(TemperatureValue(12.0f));
    msg.setCellMaxTemperature(TemperatureValue(15.0f));
    
    // Should allow extreme negative values
    tempOverride->setOverride(TemperatureValue(-50.0f), TemperatureValue(-40.0f), 1);
    bool applied = tempOverride->applyOverride(msg);
    
    CHECK_TRUE(applied);
    DOUBLES_EQUAL(-50.0f, msg.getCellMinTemperature().celsius(), 0.1f);
    DOUBLES_EQUAL(-40.0f, msg.getCellMaxTemperature().celsius(), 0.1f);
}

TEST(CanMessage374Override, FrameCountDecrementsEvenWhenRejected)
{
    CanMessage374 msg(&frame);
    
    // Set current temperature to 30°C
    msg.setCellMaxTemperature(TemperatureValue(30.0f));
    
    // Try to override with temp below 25°C, should be rejected
    tempOverride->setOverride(TemperatureValue(20.0f), TemperatureValue(24.0f), 3);
    CHECK_EQUAL(3, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(2, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(1, tempOverride->getRemainingFrames());
    
    tempOverride->applyOverride(msg);
    CHECK_EQUAL(0, tempOverride->getRemainingFrames());
    CHECK_FALSE(tempOverride->isOverrideActive());
}
