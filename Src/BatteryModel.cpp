/**
 * @file BatteryModel.cpp
 * @brief Implementation of BatteryModel class
 */

#include "BatteryModel.h"

// Voltage threshold constants for SoC2 calculation
static const VoltageByte v275 = VoltageByte::fromVoltage(2.75f);
static const VoltageByte v300 = VoltageByte::fromVoltage(3.00f);
static const VoltageByte v347 = VoltageByte::fromVoltage(3.47f);
static const VoltageByte v360 = VoltageByte::fromVoltage(3.60f);
static const VoltageByte v372 = VoltageByte::fromVoltage(3.72f);
static const VoltageByte v381 = VoltageByte::fromVoltage(3.81f);
static const VoltageByte v392 = VoltageByte::fromVoltage(3.92f);
static const VoltageByte v400 = VoltageByte::fromVoltage(4.00f);
static const VoltageByte v420 = VoltageByte::fromVoltage(4.20f);

BatteryModel::BatteryModel(float capacity)
    : m_capacity(capacity), m_remAh1(capacity), m_remAh2(capacity), m_restTimeMs(0), m_initialized(false), m_validDataCounter(0), m_vMin(VoltageByte::fromVoltage(2.76f))
{
}

void BatteryModel::update(VoltageByte cellMinVoltage, float packCurrent, uint32_t deltaTMs)
{
    // Only update vMin if value is within valid range
    if (cellMinVoltage < VoltageByte::getMinVoltage() ||
        cellMinVoltage > VoltageByte::getMaxVoltage())
    {
        return; // Ignore invalid voltage readings
    }
    // Initialize on first valid data
    m_vMin = cellMinVoltage;
    if (!m_initialized)
    {
        m_validDataCounter++;
        if (m_validDataCounter >= INIT_FRAMES_REQUIRED)
        {
            // Initialize SoC2 based on voltage
            float SoC2 = voltageToSoC2(m_vMin);
            m_remAh1 = (SoC2 * m_capacity) / 100.0f;
            m_remAh2 = m_remAh1;
            m_initialized = true;
        }
        return; // Don't process further until initialized
    }

    // Calculate amp-hours in/out during this update period
    float Ah = (packCurrent * deltaTMs) / (static_cast<float>(MILLISECONDS_PER_HOUR)); // Convert A*ms to Ah
    // Update both remaining capacity estimates
    m_remAh1 += Ah;
    m_remAh2 += Ah;

    // Clamp remAh1 to valid range
    m_remAh1 = clampRemainingAh(m_remAh1);

    // Track rest time (when current is low)
    if (packCurrent > -CURRENT_THRESHOLD && packCurrent < CURRENT_THRESHOLD)
    {
        m_restTimeMs += deltaTMs;
    }
    else
    {
        m_restTimeMs = 0;
    }

    // If battery has been at rest long enough, recalibrate SoC2 based on voltage
    if (m_restTimeMs > REST_TIME_THRESHOLD)
    {
        float SoC2 = voltageToSoC2(m_vMin);
        m_remAh2 = (SoC2 * m_capacity) / 100.0f;
    }
    else
    {
        // Clamp remAh2 to valid range
        m_remAh2 = clampRemainingAh(m_remAh2);
    }
}

float BatteryModel::getSoC1() const
{
    if (!m_initialized)
    {
        return 0.0f;
    }
    return calculateSoC(m_remAh1);
}

float BatteryModel::getSoC2() const
{
    if (!m_initialized)
    {
        return 0.0f;
    }
    return calculateSoC(m_remAh2);
}

float BatteryModel::getRemainingAh1() const
{
    return m_remAh1;
}

float BatteryModel::getRemainingAh2() const
{
    return m_remAh2;
}

float BatteryModel::getCapacity() const
{
    return m_capacity;
}

bool BatteryModel::isInitialized() const
{
    return m_initialized;
}

void BatteryModel::reset()
{
    m_remAh1 = m_capacity;
    m_remAh2 = m_capacity;
    m_restTimeMs = 0;
    m_initialized = false;
    m_validDataCounter = 0;
    m_vMin = VoltageByte::fromVoltage(2.76f);
}

float BatteryModel::calculateSoC(float remAh) const
{
    return (100.0f * remAh) / m_capacity;
}

float BatteryModel::clampRemainingAh(float remAh) const
{
    if (remAh > m_capacity)
    {
        return m_capacity;
    }
    if (remAh < 0.0f)
    {
        return 0.0f;
    }
    return remAh;
}

float BatteryModel::voltageToSoC2(VoltageByte cellMinVoltage)
{
    uint8_t value = cellMinVoltage.get();
    float soc;

    if (cellMinVoltage < v275)
    {
        soc = 0.0f;
    }
    else if (cellMinVoltage < v300)
    {
        // Simplified: 0.04082 * value + (0.04082 * 210) - 11.2255
        soc = 0.04082f * value - 2.6533f;
    }
    else if (cellMinVoltage < v347)
    {
        // Simplified: 0.33497 * value + (0.33497 * 210) - 99.471
        soc = 0.33497f * value - 29.1273f;
    }
    else if (cellMinVoltage < v360)
    {
        // Simplified: 1.32143 * value + (1.32143 * 210) - 441.573
        soc = 1.32143f * value - 164.0727f;
    }
    else if (cellMinVoltage < v372)
    {
        // Simplified: 1.83199 * value + (1.83199 * 210) - 625.684
        soc = 1.83199f * value - 241.0661f;
    }
    else if (cellMinVoltage < v381)
    {
        // Simplified: 0.89213 * value + (0.89213 * 210) - 275.962
        soc = 0.89213f * value - 88.6147f;
    }
    else if (cellMinVoltage < v392)
    {
        // Simplified: 1.31098 * value + (1.31098 * 210) - 435.5
        soc = 1.31098f * value - 160.1942f;
    }
    else if (cellMinVoltage < v400)
    {
        // Simplified: 1.00031 * value + (1.00031 * 210) - 313.686
        soc = 1.00031f * value - 103.6209f;
    }
    else if (cellMinVoltage < v420)
    {
        // Simplified:
        soc = 1.35913f * value - 171.6887f;
    }
    else
    {
        soc = 113.727f; // Maximum value (slightly above 100%)
    }

    // Clamp to [0, 100] range
    if (soc < 0.0f)
        soc = 0.0f;
    if (soc > 100.0f)
        soc = 100.0f;

    return soc;
}
