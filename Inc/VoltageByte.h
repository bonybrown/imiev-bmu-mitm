/**
 * @file VoltageByte.h
 * @brief Strong typedef class for voltage byte values
 * 
 * This class provides type safety for voltage values stored as bytes,
 * preventing accidental mixing with raw uint8_t values.
 */

#ifndef VOLTAGEBYTE_H
#define VOLTAGEBYTE_H

#include <stdint.h>

#ifdef __cplusplus

/**
 * @class VoltageByte
 * @brief Type-safe wrapper for voltage values stored as uint8_t
 * 
 * This class prevents implicit conversions between uint8_t and voltage values,
 * forcing explicit conversions and improving code safety.
 * 
 * Voltage encoding: voltage_byte = (voltage * 100) - VOLTAGE_OFFSET
 * where VOLTAGE_OFFSET = 210
 */
class VoltageByte {
private:
    uint8_t value_;

public:
    /// Voltage offset constant used in conversion formula
    static constexpr int VOLTAGE_OFFSET = 210;
    
    /// Minimum valid voltage byte value (corresponds to ~2.1V)
    static constexpr uint8_t MIN_VALUE = 0;
    
    /// Maximum valid voltage byte value (corresponds to ~4.65V)
    static constexpr uint8_t MAX_VALUE = 255;
    
    /**
     * @brief Default constructor - initializes to minimum value
     */
    constexpr VoltageByte() : value_(MIN_VALUE) {}
    
    /**
     * @brief Explicit constructor from uint8_t
     * @param val Raw byte value
     * 
     * Explicit keyword prevents implicit conversions like:
     * VoltageByte v = 100; // ERROR
     */
    explicit constexpr VoltageByte(uint8_t val) : value_(val) {}
    
    /**
     * @brief Create VoltageByte from voltage in volts
     * @param voltage Voltage value in volts (e.g., 3.7)
     * @return VoltageByte object representing the voltage
     */
    static VoltageByte fromVoltage(float voltage) {
        int raw = static_cast<int>(voltage * 100.0f) - VOLTAGE_OFFSET;
        // Clamp to valid range
        if (raw < MIN_VALUE) raw = MIN_VALUE;
        if (raw > MAX_VALUE) raw = MAX_VALUE;
        return VoltageByte(static_cast<uint8_t>(raw));
    }

    /**
     * @brief Convert to voltage in volts
     * @return Voltage value in volts
     */
    constexpr float toVoltage() const {
        return (static_cast<float>(value_) + VOLTAGE_OFFSET) / 100.0f;
    }
    
    /**
     * @brief Explicit conversion to uint8_t
     * @return Raw byte value
     * 
     * Usage: uint8_t raw = static_cast<uint8_t>(voltage_byte);
     */
    explicit constexpr operator uint8_t() const { 
        return value_; 
    }
    
    /**
     * @brief Get raw byte value
     * @return Raw byte value
     * 
     * Alternative to casting: uint8_t raw = voltage_byte.get();
     */
    constexpr uint8_t get() const { 
        return value_; 
    }

    static VoltageByte getMaxVoltage();
    static VoltageByte getMinVoltage();

    // Comparison operators
    constexpr bool operator==(VoltageByte other) const { 
        return value_ == other.value_; 
    }
    
    constexpr bool operator!=(VoltageByte other) const { 
        return value_ != other.value_; 
    }
    
    constexpr bool operator<(VoltageByte other) const { 
        return value_ < other.value_; 
    }
    
    constexpr bool operator>(VoltageByte other) const { 
        return value_ > other.value_; 
    }
    
    constexpr bool operator<=(VoltageByte other) const { 
        return value_ <= other.value_; 
    }
    
    constexpr bool operator>=(VoltageByte other) const { 
        return value_ >= other.value_; 
    }
};

#endif // __cplusplus

#endif // VOLTAGEBYTE_H
