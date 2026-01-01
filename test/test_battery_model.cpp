/**
 * @file test_battery_model.cpp
 * @brief Unit tests for BatteryModel class
 */

#include "CppUTest/TestHarness.h"
#include "BatteryModel.h"
#include "VoltageByte.h"

TEST_GROUP(BatteryModel_VoltageToSoC2){
    void setup(){}

    void teardown(){}};

TEST(BatteryModel_VoltageToSoC2, KnownVoltageTable)
{
    // Test table of known voltage to SoC conversions
    // Based on the storeSoC2() piecewise linear interpolation function

    struct TestCase
    {
        float voltage;
        uint8_t expectedByte;
        float expectedSoC;
        float tolerance;
    };

    TestCase testCases[] = {
        // Voltage, Expected Byte, Expected SoC, Tolerance
        {2.50f, 40, 0.00f, 0.1f},    // Below minimum
        {2.75f, 65, 0.00f, 0.1f},    // At minimum threshold
        {3.00f, 90, 1.02f, 0.5f},    // Low range
        {3.20f, 110, 7.72f, 0.5f},   // Low-mid range
        {3.47f, 137, 16.96f, 0.5f},  // End of low-mid range
        {3.50f, 140, 20.93f, 0.5f},  // Mid range start
        {3.65f, 155, 42.99f, 0.5f},  // Mid range
        {3.70f, 160, 52.15f, 0.5f},  // Nominal voltage
        {3.75f, 165, 58.59f, 0.5f},  // High range
        {3.85f, 175, 69.23f, 0.5f},  // Near full
        {3.90f, 180, 75.78f, 0.5f},  // Almost full
        {3.95f, 185, 81.44f, 0.5f},  // Very high
        {4.00f, 190, 86.55f, 0.5f},  // Full range start
        {4.10f, 200, 100.00f, 0.1f}, // Should be clamped to 100
        {4.20f, 210, 100.00f, 0.1f}, // Maximum voltage
        {4.50f, 240, 100.00f, 0.1f}, // Over voltage (clamped)
    };

    for (size_t i = 0; i < sizeof(testCases) / sizeof(TestCase); i++)
    {
        TestCase tc = testCases[i];

        VoltageByte vb = VoltageByte::fromVoltage(tc.voltage);

        // Verify byte conversion (allow ±1 for float rounding)
        CHECK(vb.get() >= tc.expectedByte - 1 && vb.get() <= tc.expectedByte + 1);

        // Verify SoC calculation
        float actualSoC = BatteryModel::voltageToSoC2(vb);
        DOUBLES_EQUAL(tc.expectedSoC, actualSoC, tc.tolerance);
    }
}

TEST(BatteryModel_VoltageToSoC2, Monotonicity)
{
    // Verify SoC increases with voltage (monotonic function)
    float prev_soc = 0.0f;

    for (float voltage = 2.75f; voltage <= 4.20f; voltage += 0.05f)
    {
        VoltageByte v = VoltageByte::fromVoltage(voltage);
        float soc = BatteryModel::voltageToSoC2(v);

        // SoC should never decrease as voltage increases
        CHECK(soc >= prev_soc);
        prev_soc = soc;
    }
}

TEST(BatteryModel_VoltageToSoC2, BoundaryClamping)
{
    // Verify output is always in [0, 100] range for all possible byte values
    for (uint16_t raw = 0; raw <= 255; raw++)
    {
        VoltageByte v(static_cast<uint8_t>(raw));
        float soc = BatteryModel::voltageToSoC2(v);

        CHECK(soc >= 0.0f);
        CHECK(soc <= 100.0f);
    }
}

TEST_GROUP(BatteryModel_Construction){
    void setup(){}

    void teardown(){}};

TEST(BatteryModel_Construction, DefaultConstruction)
{
    BatteryModel model(90.0f);

    // Should not be initialized yet
    CHECK_FALSE(model.isInitialized());
    DOUBLES_EQUAL(90.0f, model.getCapacity(), 0.01f);
    DOUBLES_EQUAL(90.0f, model.getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(90.0f, model.getRemainingAh2(), 0.01f);
}

TEST_GROUP(BatteryModel_Initialization)
{
    BatteryModel *model;

    void setup()
    {
        model = new BatteryModel(90.0f);
    }

    void teardown()
    {
        delete model;
    }
};

TEST(BatteryModel_Initialization, RequiresMultipleUpdates)
{
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);

    // First 19 updates should not initialize
    for (int i = 0; i < 19; i++)
    {
        model->update(voltage, 0.0f, 10);
        CHECK_FALSE(model->isInitialized());
    }

    // 20th update should initialize
    model->update(voltage, 0.0f, 10);
    CHECK(model->isInitialized());
}

TEST(BatteryModel_Initialization, InitializesFromVoltage)
{
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f); // Should give ~52% SoC

    // Perform 20 updates to initialize
    for (int i = 0; i < 20; i++)
    {
        model->update(voltage, 0.0f, 10);
    }

    CHECK(model->isInitialized());

    // SoC should be initialized from voltage
    float expectedSoC = BatteryModel::voltageToSoC2(voltage);
    DOUBLES_EQUAL(expectedSoC, model->getSoC1(), 1.0f);
    DOUBLES_EQUAL(expectedSoC, model->getSoC2(), 1.0f);
}

TEST(BatteryModel_Initialization, DoesNotInitialiseWithOutOfRangeVoltage)
{
    VoltageByte voltage = VoltageByte::fromVoltage(2.20f); // Below minimum 2.75V

    // Perform 20 updates to initialize
    for (int i = 0; i < 20; i++)
    {
        model->update(voltage, 0.0f, 10);
    }
    
    // Model should still not be initialized. Data is out of range.
    CHECK(!model->isInitialized());

    voltage = VoltageByte::fromVoltage(4.00f); // Valid voltage
    // Perform 20 updates to initialize
    for (int i = 0; i < 20; i++)
    {
        model->update(voltage, 0.0f, 10);
    }

    // SoC should be initialized from voltage
    float expectedSoC = BatteryModel::voltageToSoC2(voltage);
    DOUBLES_EQUAL(expectedSoC, model->getSoC1(), 1.0f);
    DOUBLES_EQUAL(expectedSoC, model->getSoC2(), 1.0f);
}

TEST_GROUP(BatteryModel_CoulombCounting)
{
    BatteryModel *model;

    void setup()
    {
        model = new BatteryModel(90.0f);

        // Initialize model at nominal voltage (3.70V ~ 52% SoC)
        VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
        for (int i = 0; i < 20; i++)
        {
            model->update(voltage, 0.0f, 10);
        }
        CHECK(model->isInitialized());
    }

    void teardown()
    {
        delete model;
    }
};

TEST(BatteryModel_CoulombCounting, DischargingReducesCapacity)
{
    float initialAh = model->getRemainingAh1();

    // Discharge at 10A for 1 second (10 updates of 100ms each)
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 10; i++)
    {
        model->update(voltage, -10.0f, 100);
    }

    float finalAh = model->getRemainingAh1();

    // Should have discharged approximately 10A * 1s = 10/3600 Ah = 0.00278 Ah
    DOUBLES_EQUAL(initialAh - 0.00278f, finalAh, 0.001f);
}

TEST(BatteryModel_CoulombCounting, ChargingIncreasesCapacity)
{
    float initialAh = model->getRemainingAh1();

    // Charge at 20A for 1 hour (36000 updates of 100ms each)
    VoltageByte voltage = VoltageByte::fromVoltage(3.90f);
    for (int i = 0; i < 36000; i++)
    {
        model->update(voltage, 20.0f, 100);
    }

    float finalAh = model->getRemainingAh1();

    // Should have charged approximately 20A * 1 hour = 20 Ah
    DOUBLES_EQUAL(initialAh + 20.0f, finalAh, 0.1f);
}

TEST(BatteryModel_CoulombCounting, ClampsToMaxCapacity)
{
    // Charge at high current to exceed capacity
    // Starting from ~47 Ah (52% SoC), need to add >43 Ah to exceed 90 Ah
    // At 100A for 100ms per update: 0.00278 Ah per update
    // Need 43/0.00278 = ~15,470 updates, but use more to ensure we exceed
    VoltageByte voltage = VoltageByte::fromVoltage(4.20f);
    for (int i = 0; i < 20000; i++)
    {
        model->update(voltage, 100.0f, 100);
    }

    // Should clamp to capacity
    DOUBLES_EQUAL(90.0f, model->getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(100.0f, model->getSoC1(), 0.1f);
}

TEST(BatteryModel_CoulombCounting, ClampsToZero)
{
    // Discharge at high current to go below zero
    // Starting from ~47 Ah (52% SoC), need to subtract >47 Ah to go below 0
    // At -100A for 100ms per update: -0.00278 Ah per update
    // Need 47/0.00278 = ~16,906 updates, but use more to ensure we go below zero
    VoltageByte voltage = VoltageByte::fromVoltage(2.75f);
    for (int i = 0; i < 20000; i++)
    {
        model->update(voltage, -100.0f, 100);
    }

    // Should clamp to zero
    DOUBLES_EQUAL(0.0f, model->getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(0.0f, model->getSoC1(), 0.1f);
}

TEST_GROUP(BatteryModel_VoltageCalibration)
{
    BatteryModel *model;

    void setup()
    {
        model = new BatteryModel(90.0f);

        // Initialize model at nominal voltage
        VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
        for (int i = 0; i < 20; i++)
        {
            model->update(voltage, 0.0f, 10);
        }
        CHECK(model->isInitialized());
    }

    void teardown()
    {
        delete model;
    }
};

TEST(BatteryModel_VoltageCalibration, RecalibratesAfterRest)
{
    // Simulate discharge to drift SoC2
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 100; i++)
    {
        model->update(voltage, -50.0f, 100);
    }

    float socBeforeRest = model->getSoC2();

    // Now let battery rest at higher voltage for > 60 seconds
    VoltageByte restVoltage = VoltageByte::fromVoltage(3.90f);
    for (int i = 0; i < 610; i++)
    {                                          // 61 seconds
        model->update(restVoltage, 0.5f, 100); // Very low current
    }

    float socAfterRest = model->getSoC2();

    // SoC2 should have recalibrated based on voltage
    float expectedSoC = BatteryModel::voltageToSoC2(restVoltage);
    DOUBLES_EQUAL(expectedSoC, socAfterRest, 2.0f);

    // Should be different from before rest
    CHECK(socAfterRest != socBeforeRest);
}

TEST(BatteryModel_VoltageCalibration, NoRecalibrationWithHighCurrent)
{
    // Get initial SoC2
    float initialSoC2 = model->getSoC2();

    // Update with high current for long time (should not recalibrate)
    VoltageByte voltage = VoltageByte::fromVoltage(3.90f);
    for (int i = 0; i < 610; i++)
    {                                        // 61 seconds
        model->update(voltage, -10.0f, 100); // High current
    }

    // SoC2 should have changed due to coulomb counting, not voltage calibration
    // (If it calibrated to voltage, it would be ~75%, but with discharge it should be lower)
    float finalSoC2 = model->getSoC2();
    float voltageBasedSoC = BatteryModel::voltageToSoC2(voltage);

    // Should NOT match voltage-based SoC (proving no recalibration occurred)
    CHECK(finalSoC2 < voltageBasedSoC - 5.0f);
}

TEST_GROUP(BatteryModel_Reset){
    void setup(){}

    void teardown(){}};

TEST(BatteryModel_Reset, ResetsToUninitializedState)
{
    BatteryModel model(90.0f);

    // Initialize and modify
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 20; i++)
    {
        model.update(voltage, 0.0f, 10);
    }
    CHECK(model.isInitialized());

    // Discharge
    for (int i = 0; i < 100; i++)
    {
        model.update(voltage, -10.0f, 100);
    }

    // Reset
    model.reset();

    // Should be uninitialized again
    CHECK_FALSE(model.isInitialized());
    DOUBLES_EQUAL(90.0f, model.getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(90.0f, model.getRemainingAh2(), 0.01f);
}

TEST_GROUP(BatteryModel_LongTermAccuracy){
    void setup(){}

    void teardown(){}};

TEST(BatteryModel_LongTermAccuracy, ZeroNetCurrentPreservesSoC)
{
    // This test simulates a long period with equal charge and discharge
    // The final SoC should match the initial SoC since net Ah = 0

    BatteryModel model(90.0f);

    // Initialize model at nominal voltage (3.70V ~ 52% SoC)
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 20; i++)
    {
        model.update(voltage, 0.0f, 10);
    }
    CHECK(model.isInitialized());

    // Record initial state
    float initialSoC1 = model.getSoC1();
    float initialSoC2 = model.getSoC2();
    float initialAh1 = model.getRemainingAh1();
    float initialAh2 = model.getRemainingAh2();

    // Simulate 1 hour of alternating charge/discharge cycles
    // Each cycle: 1 minute charge at 10A, 1 minute discharge at -10A
    // This gives: 10A * 60s = 600 A·s = 0.1667 Ah per charge phase
    //            -10A * 60s = -600 A·s = -0.1667 Ah per discharge phase
    // Net over each cycle = 0 Ah

    int cycles = 30; // 30 cycles = 1 hour

    for (int cycle = 0; cycle < cycles; cycle++)
    {
        // Charge phase: 10A for 60 seconds (60 updates of 1000ms each)
        for (int i = 0; i < 60; i++)
        {
            model.update(voltage, 10.0f, 1000);
        }

        // Discharge phase: -10A for 60 seconds (60 updates of 1000ms each)
        for (int i = 0; i < 60; i++)
        {
            model.update(voltage, -10.0f, 1000);
        }
    }

    // Check final state matches initial state
    float finalSoC1 = model.getSoC1();
    float finalSoC2 = model.getSoC2();
    float finalAh1 = model.getRemainingAh1();
    float finalAh2 = model.getRemainingAh2();

    // Verify SoC1 (coulomb counting) is preserved
    DOUBLES_EQUAL(initialSoC1, finalSoC1, 0.01f);
    DOUBLES_EQUAL(initialAh1, finalAh1, 0.001f);

    // Verify SoC2 (coulomb counting, no voltage recalibration due to high current)
    DOUBLES_EQUAL(initialSoC2, finalSoC2, 0.01f);
    DOUBLES_EQUAL(initialAh2, finalAh2, 0.001f);
}

TEST(BatteryModel_LongTermAccuracy, ZeroNetRandomCurrentPreservesSoC)
{
    // This test simulates a long period with equal charge and discharge
    // The final SoC should match the initial SoC since net Ah = 0

    BatteryModel model(90.0f);

    // Initialize model at nominal voltage (3.70V ~ 52% SoC)
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 20; i++)
    {
        model.update(voltage, 0.0f, 10);
    }
    CHECK(model.isInitialized());

    // Record initial state
    float initialSoC1 = model.getSoC1();
    float initialSoC2 = model.getSoC2();
    float initialAh1 = model.getRemainingAh1();
    float initialAh2 = model.getRemainingAh2();

    // Simulate 10 hours of random charge/discharge flows
    const int cycles = 10 * 60 * 60 * 10; // 10 hours x 60 mins x 60 seconds x 100ms per update (360000 updates)
    float values[cycles] = {0};
    for (int cycle = 0; cycle < cycles; cycle++)
    {
        float current = (rand() % 2001 - 1000) / 100.0f; // Random current between -10A and +10A
        // ensure each value is more than 2A in magnitude to avoid voltage recalibration
        if (current > 0)
        {
            values[cycle] = current + 5; // Charge
        }
        else
        {
            values[cycle] = current - 5; // Discharge
        }
    }

    // Now apply the generated current values
    for (int cycle = 0; cycle < cycles; cycle++)
    {
        float current = values[cycle];
        model.update(voltage, current, 100);
    }
    // Now reverse the process to ensure zero net current
    for (int cycle = 0; cycle < cycles; cycle++)
    {
        float current = -values[cycle];
        model.update(voltage, current, 100);
    }

    // Check final state matches initial state
    float finalSoC1 = model.getSoC1();
    float finalSoC2 = model.getSoC2();
    float finalAh1 = model.getRemainingAh1();
    float finalAh2 = model.getRemainingAh2();

    // Verify SoC1 (coulomb counting) is preserved
    DOUBLES_EQUAL(initialSoC1, finalSoC1, 0.01f);
    DOUBLES_EQUAL(initialAh1, finalAh1, 0.001f);

    // Verify SoC2 (coulomb counting, no voltage recalibration due to high current)
    DOUBLES_EQUAL(initialSoC2, finalSoC2, 0.01f);
    DOUBLES_EQUAL(initialAh2, finalAh2, 0.001f);
}

TEST(BatteryModel_LongTermAccuracy, AsymmetricCyclesWithZeroNet)
{
    // This test uses different current magnitudes but equal total Ah transfer
    // Initial: ~47 Ah (52% of 90 Ah)
    // Add 5 Ah, then remove 5 Ah over multiple cycles

    BatteryModel model(90.0f);

    // Initialize at 3.70V
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 20; i++)
    {
        model.update(voltage, 0.0f, 10);
    }
    CHECK(model.isInitialized());

    float initialAh1 = model.getRemainingAh1();
    float initialSoC1 = model.getSoC1();

    // Cycle 1: Charge 2.5 Ah with 50A for 3 minutes
    // 50A * 180s = 9000 A·s = 2.5 Ah
    for (int i = 0; i < 180; i++)
    {
        model.update(voltage, 50.0f, 1000);
    }

    // Cycle 2: Charge 2.5 Ah with 25A for 6 minutes
    // 25A * 360s = 9000 A·s = 2.5 Ah
    for (int i = 0; i < 360; i++)
    {
        model.update(voltage, 25.0f, 1000);
    }

    // Total charged: 5 Ah
    // Now discharge 5 Ah in different pattern

    // Cycle 3: Discharge 3 Ah with -30A for 6 minutes
    // -30A * 360s = -10800 A·s = -3 Ah
    for (int i = 0; i < 360; i++)
    {
        model.update(voltage, -30.0f, 1000);
    }

    // Cycle 4: Discharge 2 Ah with -20A for 6 minutes
    // -20A * 360s = -7200 A·s = -2 Ah
    for (int i = 0; i < 360; i++)
    {
        model.update(voltage, -20.0f, 1000);
    }

    // Total discharged: 5 Ah
    // Net change: 0 Ah

    float finalAh1 = model.getRemainingAh1();
    float finalSoC1 = model.getSoC1();

    // Verify coulomb counting accuracy
    DOUBLES_EQUAL(initialAh1, finalAh1, 0.01f);
    DOUBLES_EQUAL(initialSoC1, finalSoC1, 0.1f);
}
