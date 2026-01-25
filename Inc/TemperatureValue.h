#pragma once
#include <cstdint>

/**
 * @brief Value class for temperature conversion between float (Celsius) and CAN byte (offset by 50)
 */

class TemperatureValue {
public:
    TemperatureValue() : m_celsius(0.0f) {}
    explicit TemperatureValue(float celsius) : m_celsius(celsius) {}
    explicit TemperatureValue(uint8_t canByte) : m_celsius(fromCanByte(canByte)) {}

    // Instance method: get CAN byte representation of stored value
    uint8_t toCanByte() const {
        int value = static_cast<int>(m_celsius + 50.0f);
        if (value < 0) value = 0;
        if (value > 255) value = 255;
        return static_cast<uint8_t>(value);
    }

    // Instance method: set from CAN byte
    void setFromCanByte(uint8_t canByte) {
        m_celsius = fromCanByte(canByte);
    }

    // Static utility: convert float Celsius to CAN byte
    static uint8_t toCanByte(float celsius) {
        int value = static_cast<int>(celsius + 50.0f);
        if (value < 0) value = 0;
        if (value > 255) value = 255;
        return static_cast<uint8_t>(value);
    }

    // Static utility: convert CAN byte to float Celsius
    static float fromCanByte(uint8_t canByte) {
        return static_cast<float>(canByte) - 50.0f;
    }

    // Accessors
    float celsius() const { return m_celsius; }
    void setCelsius(float celsius) { m_celsius = celsius; }


    // Comparison operators
    bool operator==(const TemperatureValue& other) const { return m_celsius == other.m_celsius; }
    bool operator!=(const TemperatureValue& other) const { return m_celsius != other.m_celsius; }
    bool operator<(const TemperatureValue& other) const { return m_celsius < other.m_celsius; }
    bool operator>(const TemperatureValue& other) const { return m_celsius > other.m_celsius; }
    bool operator<=(const TemperatureValue& other) const { return m_celsius <= other.m_celsius; }
    bool operator>=(const TemperatureValue& other) const { return m_celsius >= other.m_celsius; }

private:
    float m_celsius;
};
