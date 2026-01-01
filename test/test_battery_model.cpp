/**
 * @file test_battery_model.cpp
 * @brief Unit tests for BatteryModel class
 */

#include "CppUTest/TestHarness.h"
#include "BatteryModel.h"
#include "VoltageByte.h"

TEST_GROUP(BatteryModel_VoltageToSoC2)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(BatteryModel_VoltageToSoC2, KnownVoltageTable)
{
    // Test table of known voltage to SoC conversions
    // Based on the storeSoC2() piecewise linear interpolation function
    
    struct TestCase {
        float voltage;
        uint8_t expectedByte;
        float expectedSoC;
        float tolerance;
    };
    
    TestCase testCases[] = {
        // Voltage, Expected Byte, Expected SoC, Tolerance
        {2.50f,   40,    0.00f,  0.1f},  // Below minimum
        {2.75f,   65,    0.00f,  0.1f},  // At minimum threshold
        {3.00f,   90,    1.02f,  0.5f},  // Low range
        {3.20f,  110,    7.72f,  0.5f},  // Low-mid range
        {3.47f,  137,   16.96f,  0.5f},  // End of low-mid range
        {3.50f,  140,   20.93f,  0.5f},  // Mid range start
        {3.65f,  155,   42.99f,  0.5f},  // Mid range
        {3.70f,  160,   52.15f,  0.5f},  // Nominal voltage
        {3.75f,  165,   58.59f,  0.5f},  // High range
        {3.85f,  175,   69.23f,  0.5f},  // Near full
        {3.90f,  180,   75.78f,  0.5f},  // Almost full
        {3.95f,  185,   81.44f,  0.5f},  // Very high
        {4.00f,  190,   86.55f,  0.5f},  // Full range start
        {4.10f,  200,  100.00f,  0.1f},  // Should be clamped to 100
        {4.20f,  210,  100.00f,  0.1f},  // Maximum voltage
        {4.50f,  240,  100.00f,  0.1f},  // Over voltage (clamped)
    };
    
    for (size_t i = 0; i < sizeof(testCases) / sizeof(TestCase); i++) {
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
    
    for (float voltage = 2.75f; voltage <= 4.20f; voltage += 0.05f) {
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
    for (uint16_t raw = 0; raw <= 255; raw++) {
        VoltageByte v(static_cast<uint8_t>(raw));
        float soc = BatteryModel::voltageToSoC2(v);
        
        CHECK(soc >= 0.0f);
        CHECK(soc <= 100.0f);
    }
}

TEST_GROUP(BatteryModel_Construction)
{
    void setup() {
    }
    
    void teardown() {
    }
};

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
    BatteryModel* model;
    
    void setup() {
        model = new BatteryModel(90.0f);
    }
    
    void teardown() {
        delete model;
    }
};

TEST(BatteryModel_Initialization, RequiresMultipleUpdates)
{
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    
    // First 19 updates should not initialize
    for (int i = 0; i < 19; i++) {
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
    for (int i = 0; i < 20; i++) {
        model->update(voltage, 0.0f, 10);
    }
    
    CHECK(model->isInitialized());
    
    // SoC should be initialized from voltage
    float expectedSoC = BatteryModel::voltageToSoC2(voltage);
    DOUBLES_EQUAL(expectedSoC, model->getSoC1(), 1.0f);
    DOUBLES_EQUAL(expectedSoC, model->getSoC2(), 1.0f);
}

TEST_GROUP(BatteryModel_CoulombCounting)
{
    BatteryModel* model;
    
    void setup() {
        model = new BatteryModel(90.0f);
        
        // Initialize model at nominal voltage (3.70V ~ 52% SoC)
        VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
        for (int i = 0; i < 20; i++) {
            model->update(voltage, 0.0f, 10);
        }
        CHECK(model->isInitialized());
    }
    
    void teardown() {
        delete model;
    }
};

TEST(BatteryModel_CoulombCounting, DischargingReducesCapacity)
{
    float initialAh = model->getRemainingAh1();
    
    // Discharge at 10A for 1 second (10 updates of 100ms each)
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 10; i++) {
        model->update(voltage, -10.0f, 100);
    }
    
    float finalAh = model->getRemainingAh1();
    
    // Should have discharged approximately 10A * 1s = 10/3600 Ah = 0.00278 Ah
    DOUBLES_EQUAL(initialAh - 0.00278f, finalAh, 0.001f);
}

TEST(BatteryModel_CoulombCounting, ChargingIncreasesCapacity)
{
    float initialAh = model->getRemainingAh1();
    
    // Charge at 20A for 1 second (10 updates of 100ms each)
    VoltageByte voltage = VoltageByte::fromVoltage(3.90f);
    for (int i = 0; i < 10; i++) {
        model->update(voltage, 20.0f, 100);
    }
    
    float finalAh = model->getRemainingAh1();
    
    // Should have charged approximately 20A * 1s = 20/3600 Ah = 0.00556 Ah
    DOUBLES_EQUAL(initialAh + 0.00556f, finalAh, 0.001f);
}

TEST(BatteryModel_CoulombCounting, ClampsToMaxCapacity)
{
    // Charge at high current to exceed capacity
    // Starting from ~47 Ah (52% SoC), need to add >43 Ah to exceed 90 Ah
    // At 100A for 100ms per update: 0.00278 Ah per update
    // Need 43/0.00278 = ~15,470 updates, but use more to ensure we exceed
    VoltageByte voltage = VoltageByte::fromVoltage(4.20f);
    for (int i = 0; i < 20000; i++) {
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
    for (int i = 0; i < 20000; i++) {
        model->update(voltage, -100.0f, 100);
    }
    
    // Should clamp to zero
    DOUBLES_EQUAL(0.0f, model->getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(0.0f, model->getSoC1(), 0.1f);
}

TEST_GROUP(BatteryModel_VoltageCalibration)
{
    BatteryModel* model;
    
    void setup() {
        model = new BatteryModel(90.0f);
        
        // Initialize model at nominal voltage
        VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
        for (int i = 0; i < 20; i++) {
            model->update(voltage, 0.0f, 10);
        }
        CHECK(model->isInitialized());
    }
    
    void teardown() {
        delete model;
    }
};

TEST(BatteryModel_VoltageCalibration, RecalibratesAfterRest)
{
    // Simulate discharge to drift SoC2
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 100; i++) {
        model->update(voltage, -50.0f, 100);
    }
    
    float socBeforeRest = model->getSoC2();
    
    // Now let battery rest at higher voltage for > 60 seconds
    VoltageByte restVoltage = VoltageByte::fromVoltage(3.90f);
    for (int i = 0; i < 610; i++) {  // 61 seconds
        model->update(restVoltage, 0.5f, 100);  // Very low current
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
    for (int i = 0; i < 610; i++) {  // 61 seconds
        model->update(voltage, -10.0f, 100);  // High current
    }
    
    // SoC2 should have changed due to coulomb counting, not voltage calibration
    // (If it calibrated to voltage, it would be ~75%, but with discharge it should be lower)
    float finalSoC2 = model->getSoC2();
    float voltageBasedSoC = BatteryModel::voltageToSoC2(voltage);
    
    // Should NOT match voltage-based SoC (proving no recalibration occurred)
    CHECK(finalSoC2 < voltageBasedSoC - 5.0f);
}

TEST_GROUP(BatteryModel_Reset)
{
    void setup() {
    }
    
    void teardown() {
    }
};

TEST(BatteryModel_Reset, ResetsToUninitializedState)
{
    BatteryModel model(90.0f);
    
    // Initialize and modify
    VoltageByte voltage = VoltageByte::fromVoltage(3.70f);
    for (int i = 0; i < 20; i++) {
        model.update(voltage, 0.0f, 10);
    }
    CHECK(model.isInitialized());
    
    // Discharge
    for (int i = 0; i < 100; i++) {
        model.update(voltage, -10.0f, 100);
    }
    
    // Reset
    model.reset();
    
    // Should be uninitialized again
    CHECK_FALSE(model.isInitialized());
    DOUBLES_EQUAL(90.0f, model.getRemainingAh1(), 0.01f);
    DOUBLES_EQUAL(90.0f, model.getRemainingAh2(), 0.01f);
}
