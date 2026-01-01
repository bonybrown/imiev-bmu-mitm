/**
 * @file BatteryModel.h
 * @brief Battery state of charge model
 * 
 * Implements a battery model based on coulomb counting and voltage-based SoC estimation.
 * The model maintains two SoC estimates:
 * - SoC1: Based purely on coulomb counting (amp-hour integration)
 * - SoC2: Based on voltage when battery is at rest, otherwise coulomb counting
 */

#ifndef BATTERY_MODEL_H
#define BATTERY_MODEL_H

#include <stdint.h>
#include "VoltageByte.h"

/**
 * @class BatteryModel
 * @brief Models battery state of charge using dual estimation methods
 */
class BatteryModel {
public:
    /**
     * @brief Constructor
     * @param capacity Battery capacity in amp-hours
     */
    BatteryModel(float capacity);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~BatteryModel() = default;
    
    /**
     * @brief Update model with cell voltage and pack current
     * @param cellMinVoltage Minimum cell voltage in the pack
     * @param packCurrent Pack current in amps (positive = charging, negative = discharging)
     * @param deltaTMs Time elapsed since last update in milliseconds
     */
    virtual void update(VoltageByte cellMinVoltage, float packCurrent, uint32_t deltaTMs);
    
    /**
     * @brief Get state of charge based on coulomb counting
     * @return SoC1 percentage (0-100%)
     */
    float getSoC1() const;
    
    /**
     * @brief Get state of charge based on voltage/coulomb counting hybrid
     * @return SoC2 percentage (0-100%)
     */
    float getSoC2() const;
    
    /**
     * @brief Get remaining capacity based on coulomb counting
     * @return Remaining amp-hours
     */
    float getRemainingAh1() const;
    
    /**
     * @brief Get remaining capacity based on voltage/coulomb counting hybrid
     * @return Remaining amp-hours
     */
    float getRemainingAh2() const;
    
    /**
     * @brief Get total battery capacity
     * @return Capacity in amp-hours
     */
    float getCapacity() const;
    
    /**
     * @brief Check if model has been initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Get valid data counter
     * @return Number of valid data frames received during initialization
     */
    uint8_t getValidDataCounter() const { return m_validDataCounter; }
    
    /**
     * @brief Reset the model (e.g., after power cycle)
     */
    void reset();
    
    /**
     * @brief Calculate State of Charge (SoC2) from voltage byte
     * @param cellMinVoltage Minimum cell voltage in the pack
     * @return State of Charge as percentage (0.0 to 100.0)
     *
     * This static method uses piecewise linear interpolation
     * to convert battery cell voltage to state of charge percentage.
     * The function clamps the result to [0, 100] range.
     */
    static float voltageToSoC2(VoltageByte cellMinVoltage);

private:
    float m_capacity;              ///< Battery capacity in Ah
    float m_remAh1;                ///< Remaining Ah based on coulomb counting
    float m_remAh2;                ///< Remaining Ah based on voltage/coulomb counting hybrid
    uint32_t m_restTimeMs;         ///< Time battery has been at rest (low current) in ms
    bool m_initialized;            ///< Initialization flag
    uint8_t m_validDataCounter;    ///< Counter for valid data frames during initialization
    VoltageByte m_vMin;            ///< Minimum cell voltage
    
    // Configuration constants
    static constexpr float CURRENT_THRESHOLD = 2.0f;     ///< Current threshold for "at rest" in amps
    static constexpr uint32_t REST_TIME_THRESHOLD = 60000; ///< Time threshold for voltage-based SoC in ms
    static constexpr uint8_t INIT_FRAMES_REQUIRED = 20;  ///< Number of valid frames before initialization
    
    /**
     * @brief Calculate SoC from remaining Ah
     * @param remAh Remaining amp-hours
     * @return SoC percentage (0-100%)
     */
    float calculateSoC(float remAh) const;
    
    /**
     * @brief Clamp remaining Ah to valid range
     * @param remAh Remaining amp-hours to clamp
     * @return Clamped value (0 to capacity)
     */
    float clampRemainingAh(float remAh) const;
};

#endif // BATTERY_MODEL_H
