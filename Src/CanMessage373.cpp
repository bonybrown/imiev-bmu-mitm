/**
 * @file CanMessage373.cpp
 * @brief Implementation of CanMessage373 class
 */

#include "CanMessage373.h"

/**
 * @brief Construct from CAN frame pointer
 * @param frame Pointer to CAN_FRAME structure
 */
CanMessage373::CanMessage373(const CAN_FRAME* frame)
    : frame_(const_cast<CAN_FRAME*>(frame))
{
}

/**
 * @brief Get cell maximum voltage as VoltageByte
 * @return VoltageByte representing maximum cell voltage
 */
VoltageByte CanMessage373::getCellMaxVoltage() const
{
    return VoltageByte(frame_->data[0]);
}

/**
 * @brief Get cell minimum voltage as VoltageByte
 * @return VoltageByte representing minimum cell voltage
 */
VoltageByte CanMessage373::getCellMinVoltage() const
{
    return VoltageByte(frame_->data[1]);
}

/**
 * @brief Get battery pack current in amperes
 * 
 * Current is encoded as: (D2 * 256 + D3 - 32768) / 100
 * Positive values indicate charging, negative values indicate discharging.
 * 
 * Note: Using 32700 instead of 32768 as the calibrated zero point.
 * 
 * @return Current in amps
 */
float CanMessage373::getPackCurrent() const
{
    uint8_t ah = frame_->data[2];
    uint8_t al = frame_->data[3];
    return ((ah << 8 ) + al - 32700) / 100.0f;
}

/**
 * @brief Get battery pack voltage in volts
 * 
 * Pack voltage is encoded as: (D4 * 256 + D5) / 10
 * 
 * @return Pack voltage in volts
 */
float CanMessage373::getPackVoltage() const
{
    uint8_t vh = frame_->data[4];
    uint8_t vl = frame_->data[5];
    return ((vh << 8) + vl) / 10.0f;
}

