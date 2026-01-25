/**
 * @file CanMessage374.cpp
 * @brief Implementation of CanMessage374 class
 */

#include "CanMessage374.h"
#include <TemperatureValue.h>

/**
 * @brief Construct from CAN frame pointer
 * @param frame Pointer to CAN_FRAME structure
 */
CanMessage374::CanMessage374(const CAN_FRAME *frame)
    : frame_(const_cast<CAN_FRAME *>(frame))
{
}

/**
 * @brief Get State of Charge 1 (coulomb counting based)
 *
 * SoC1 is encoded as: (D0 - 10) / 2
 *
 * @return SoC1 as percentage (0.0 to 100.0)
 */
float CanMessage374::getSoC1() const
{
    return (frame_->data[0] - 10) / 2.0f;
}

/**
 * @brief Get State of Charge 2 (voltage/coulomb counting based)
 *
 * SoC2 is encoded as: (D1 - 10) / 2
 *
 * @return SoC2 as percentage (0.0 to 100.0)
 */
float CanMessage374::getSoC2() const
{
    return (frame_->data[1] - 10) / 2.0f;
}

/**
 * @brief Get cell maximum temperature
 *
 * Max temperature is encoded as: D4 - 50
 *
 * @return Maximum temperature in degrees Celsius
 */
TemperatureValue CanMessage374::getCellMaxTemperature() const
{
    return TemperatureValue(frame_->data[4]);
}

/**
 * @brief Get cell minimum temperature
 *
 * Min temperature is encoded as: D5 - 50
 *
 * @return Minimum temperature in degrees Celsius
 */
TemperatureValue CanMessage374::getCellMinTemperature() const
{
    return TemperatureValue(frame_->data[5]);
}

/**
 * @brief Get battery 100% capacity
 *
 * Capacity is encoded as: D6 / 2
 *
 * @return Battery capacity in Amp-hours
 */
float CanMessage374::getBatteryCapacity() const
{
    return frame_->data[6] / 2.0f;
}

/**
 * @brief Get raw data byte at index
 * @param index Data byte index (0-7)
 * @return Raw data byte value
 */
uint8_t CanMessage374::getRawData(uint8_t index) const
{
    if (index > 7)
    {
        return 0;
    }
    return frame_->data[index];
}

/**
 * @brief Set State of Charge 1
 *
 * Encodes SoC1 as: D0 = SoC * 2 + 10
 * Clamps input to [0, 100] range
 *
 * @param soc SoC1 percentage (0.0 to 100.0)
 */
void CanMessage374::setSoC1(float soc)
{
    // Clamp to [0, 100] range
    if (soc < 0.0f)
        soc = 0.0f;
    if (soc > 100.0f)
        soc = 100.0f;

    int value = (int)(soc * 2.0f + 10.0f + 0.5f); // Round to nearest
    frame_->data[0] = clampToUint8(value);
}

/**
 * @brief Set State of Charge 2
 *
 * Encodes SoC2 as: D1 = SoC * 2 + 10
 * Clamps input to [0, 100] range
 *
 * @param soc SoC2 percentage (0.0 to 100.0)
 */
void CanMessage374::setSoC2(float soc)
{
    // Clamp to [0, 100] range
    if (soc < 0.0f)
        soc = 0.0f;
    if (soc > 100.0f)
        soc = 100.0f;

    int value = (int)(soc * 2.0f + 10.0f + 0.5f); // Round to nearest
    frame_->data[1] = clampToUint8(value);
}

/**
 * @brief Set cell maximum temperature
 *
 * Encodes temperature as: D4 = temp + 50
 *
 * @param temp Maximum temperature in degrees Celsius
 */
void CanMessage374::setCellMaxTemperature(TemperatureValue temp)
{
    frame_->data[4] = temp.toCanByte();
}

/**
 * @brief Set cell minimum temperature
 *
 * Encodes temperature as: D5 = temp + 50
 *
 * @param temp Minimum temperature in degrees Celsius
 */
void CanMessage374::setCellMinTemperature(TemperatureValue temp)
{
    frame_->data[5] = temp.toCanByte();
}

/**
 * @brief Set battery 100% capacity
 *
 * Encodes capacity as: D6 = capacity * 2
 *
 * @param capacity Battery capacity in Amp-hours
 */
void CanMessage374::setBatteryCapacity(float capacity)
{
    int value = (int)(capacity * 2.0f + 0.5f); // Round to nearest
    frame_->data[6] = clampToUint8(value);
}

/**
 * @brief Get pointer to underlying CAN frame
 * @return Pointer to CAN_FRAME
 */
CAN_FRAME *CanMessage374::getFrame()
{
    return frame_;
}

const CAN_FRAME *CanMessage374::getFrame() const
{
    return frame_;
}

/**
 * @brief Clamp value to uint8_t range
 * @param value Value to clamp
 * @return Clamped value in range [0, 255]
 */
uint8_t CanMessage374::clampToUint8(int value)
{
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return static_cast<uint8_t>(value);
}
