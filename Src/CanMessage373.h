/**
 * @file CanMessage373.h
 * @brief CAN message 0x373 - Battery data frame parser
 * 
 * Battery data transmitted every 10ms. Data bits:
 * D0: Battery Cell Maximum Voltage (V): (D0 + 210) / 100
 * D1: Battery Cell Minimum Voltage (V): (D1 + 210) / 100
 * D2-D3: Battery pack amps (A): (D2 * 256 + D3 - 32768) / 100
 * D4-D5: Battery pack voltage (V): (D4 * 256 + D5) / 10
 * 
 * Reference: https://github.com/KommyKT/i-miev-obd2/blob/master/README.md#236
 */

#ifndef CAN_MESSAGE_373_H
#define CAN_MESSAGE_373_H

#include <stdint.h>
#include "VoltageByte.h"
#include "can_types.h"

/**
 * @class CanMessage373
 * @brief Represents CAN message 0x373 containing battery data
 * 
 * Provides type-safe access to battery voltage and current measurements
 * transmitted by the BMU (Battery Management Unit).
 */
class CanMessage373 {
public:
    static const uint16_t MESSAGE_ID = 0x373;
    
    /**
     * @brief Construct from CAN frame pointer
     * @param frame Pointer to CAN_FRAME structure
     */
    explicit CanMessage373(const CAN_FRAME* frame);
    
    /**
     * @brief Get cell maximum voltage as VoltageByte
     * @return VoltageByte representing maximum cell voltage
     */
    VoltageByte getCellMaxVoltage() const;
    
    /**
     * @brief Get cell minimum voltage as VoltageByte
     * @return VoltageByte representing minimum cell voltage
     */
    VoltageByte getCellMinVoltage() const;
    
    /**
     * @brief Get battery pack current in amperes
     * @return Current in amps (positive = charging, negative = discharging)
     */
    float getPackCurrent() const;
    
    /**
     * @brief Get battery pack voltage in volts
     * @return Pack voltage in volts
     */
    float getPackVoltage() const;
    
    
private:
    CAN_FRAME* frame_;
};

#endif // CAN_MESSAGE_373_H
