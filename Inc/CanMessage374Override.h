#pragma once

#include <cstdint>
#include "CanMessage374.h"
#include "TemperatureValue.h"

/**
 * @brief Manages temperature override for CAN message 0x374
 * 
 * This class allows temporary overriding of min and max cell temperatures
 * in message 0x374. The override is applied for a specified number of frames,
 * after which it automatically expires.
 */
class CanMessage374Override {
public:
    CanMessage374Override();

    /**
     * @brief Set override values for cell temperatures
     * @param minTemp Minimum cell temperature override value
     * @param maxTemp Maximum cell temperature override value
     * @param frameCount Number of frames to apply the override
     */
    void setOverride(TemperatureValue minTemp, TemperatureValue maxTemp, uint32_t frameCount);

    /**
     * @brief Apply override to temperature values in CanMessage374 if active
     * @param msg Reference to CanMessage374 - temperatures will be overridden if active
     * @return true if override was applied, false otherwise
     * 
     * This method stores the original values before overriding them,
     * decrements the frame counter, and deactivates when counter reaches zero.
     * 
     * Override will NOT be applied if:
     * - Current max temp > 25°C and override max temp < 25°C (prevents unrealistic cooling)
     * - Current max temp <= 10°C and override max temp > (current + 10°C) (prevents unrealistic heating)
     */
    bool applyOverride(CanMessage374& msg);

    /**
     * @brief Check if override is currently active
     * @return true if override is active, false otherwise
     */
    bool isOverrideActive() const;

    /**
     * @brief Clear the override immediately
     */
    void clearOverride();

    /**
     * @brief Get the original min temperature from last override application
     * @return Original min temperature value
     */
    TemperatureValue getOriginalMinTemp() const;

    /**
     * @brief Get the original max temperature from last override application
     * @return Original max temperature value
     */
    TemperatureValue getOriginalMaxTemp() const;

    /**
     * @brief Get remaining number of frames for current override
     * @return Number of frames remaining
     */
    uint32_t getRemainingFrames() const;

private:
    TemperatureValue m_overrideMinTemp;
    TemperatureValue m_overrideMaxTemp;
    uint32_t m_remainingFrames;
    
    TemperatureValue m_originalMinTemp;
    TemperatureValue m_originalMaxTemp;
};
