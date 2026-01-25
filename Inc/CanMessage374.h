/**
 * @file CanMessage374.h
 * @brief CAN message 0x374 - Battery SOC and temperature frame parser
 * 
 * Battery SOC transmitted every 100ms. Data bits:
 * D0: State of charge (%): (D0 - 10) / 2 (OBDZero)
 * D1: State of charge (%): (D1 - 10) / 2
 * D4: Cell Maximum temperature (°C): D4 - 50 (OBDZero)
 * D5: Cell Minimum temperature (°C): D5 - 50 (OBDZero)
 * D6: Battery 100% capacity (Ah): D6 / 2 (OBDZero)
 * 
 * Reference: https://github.com/KommyKT/i-miev-obd2/blob/master/README.md#236
 */

#ifndef CAN_MESSAGE_374_H
#define CAN_MESSAGE_374_H

#include <stdint.h>
#include "can_types.h"
#include <TemperatureValue.h>

/**
 * @class CanMessage374
 * @brief Represents CAN message 0x374 containing battery SOC and temperature
 * 
 * Provides type-safe access to state of charge and temperature measurements
 * transmitted by the BMU (Battery Management Unit).
 */
class CanMessage374 {
public:
    static const uint16_t MESSAGE_ID = 0x374;
    static const uint8_t RECURRENCE_MS = 100; // Message sent every 100ms
    
    /**
     * @brief Construct from CAN frame pointer
     * @param frame Pointer to CAN_FRAME structure
     */
    explicit CanMessage374(const CAN_FRAME* frame);
    
    /**
     * @brief Get State of Charge 1 (coulomb counting based)
     * @return SoC1 as percentage (0.0 to 100.0)
     */
    float getSoC1() const;
    
    /**
     * @brief Get State of Charge 2 (voltage/coulomb counting based)
     * @return SoC2 as percentage (0.0 to 100.0)
     */
    float getSoC2() const;
    
    /**
     * @brief Get cell maximum temperature
     * @return Maximum temperature in degrees Celsius
     */
    TemperatureValue getCellMaxTemperature() const;
    
    /**
     * @brief Get cell minimum temperature
     * @return Minimum temperature in degrees Celsius
     */
    TemperatureValue getCellMinTemperature() const;
    
    /**
     * @brief Get battery 100% capacity
     * @return Battery capacity in Amp-hours
     */
    float getBatteryCapacity() const;
    
    /**
     * @brief Get raw data byte at index
     * @param index Data byte index (0-7)
     * @return Raw data byte value
     */
    uint8_t getRawData(uint8_t index) const;
    
    /**
     * @brief Set State of Charge 1
     * @param soc SoC1 percentage (0.0 to 100.0)
     */
    void setSoC1(float soc);
    
    /**
     * @brief Set State of Charge 2
     * @param soc SoC2 percentage (0.0 to 100.0)
     */
    void setSoC2(float soc);
    
    /**
     * @brief Set cell maximum temperature
     * @param temp Maximum temperature in degrees Celsius
     */
    void setCellMaxTemperature(TemperatureValue temp);
    
    /**
     * @brief Set cell minimum temperature
     * @param temp Minimum temperature in degrees Celsius
     */
    void setCellMinTemperature(TemperatureValue temp);
    
    /**
     * @brief Set battery 100% capacity
     * @param capacity Battery capacity in Amp-hours
     */
    void setBatteryCapacity(float capacity);
    
    /**
     * @brief Get pointer to underlying CAN frame
     * @return Pointer to CAN_FRAME
     */
    CAN_FRAME* getFrame();
    const CAN_FRAME* getFrame() const;
    
private:
    CAN_FRAME* frame_;
    
    /**
     * @brief Clamp value to uint8_t range
     * @param value Value to clamp
     * @return Clamped value in range [0, 255]
     */
    static uint8_t clampToUint8(int value);
};

#endif // CAN_MESSAGE_374_H

