#include "CanMessage374Override.h"

CanMessage374Override::CanMessage374Override()
    : m_overrideMinTemp(0.0f)
    , m_overrideMaxTemp(0.0f)
    , m_remainingFrames(0)
    , m_originalMinTemp(0.0f)
    , m_originalMaxTemp(0.0f)
{
}

void CanMessage374Override::setOverride(TemperatureValue minTemp, TemperatureValue maxTemp, uint32_t frameCount)
{
    m_overrideMinTemp = minTemp;
    m_overrideMaxTemp = maxTemp;
    m_remainingFrames = frameCount;
}

bool CanMessage374Override::applyOverride(CanMessage374& msg)
{
    if (m_remainingFrames == 0) {
        return false;
    }

    // Get current temperatures for safety checks
    float currentMaxTemp = msg.getCellMaxTemperature().celsius();

    // Safety check 1: Prevent unrealistic cooling
    // If current temp > 25°C and override < 25°C, reject
    if (currentMaxTemp > 25.0f && m_overrideMaxTemp.celsius() < 25.0f) {
        m_remainingFrames--; // Still consume frame count
        return false;
    }

    // Safety check 2: Prevent unrealistic heating from cold
    // If current temp <= 10°C and override > (current + 10°C), reject
    if (currentMaxTemp <= 10.0f && m_overrideMaxTemp.celsius() > (currentMaxTemp + 10.0f)) {
        m_remainingFrames--; // Still consume frame count
        return false;
    }

    // Store original values
    m_originalMinTemp = msg.getCellMinTemperature();
    m_originalMaxTemp = TemperatureValue(currentMaxTemp);

    // Apply override
    msg.setCellMinTemperature(m_overrideMinTemp);
    msg.setCellMaxTemperature(m_overrideMaxTemp);

    // Decrement counter
    m_remainingFrames--;

    return true;
}

bool CanMessage374Override::isOverrideActive() const
{
    return m_remainingFrames > 0;
}

void CanMessage374Override::clearOverride()
{
    m_remainingFrames = 0;
}

TemperatureValue CanMessage374Override::getOriginalMinTemp() const
{
    return m_originalMinTemp;
}

TemperatureValue CanMessage374Override::getOriginalMaxTemp() const
{
    return m_originalMaxTemp;
}

uint32_t CanMessage374Override::getRemainingFrames() const
{
    return m_remainingFrames;
}
