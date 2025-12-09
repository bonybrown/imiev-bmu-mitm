/**
 * @file VoltageByte.cpp
 * @brief Implementation of VoltageByte class methods
 */

#include "VoltageByte.h"

const VoltageByte v275 = VoltageByte::fromVoltage(2.75f);
const VoltageByte v300 = VoltageByte::fromVoltage(3.00f);
const VoltageByte v347 = VoltageByte::fromVoltage(3.47f);
const VoltageByte v360 = VoltageByte::fromVoltage(3.60f);
const VoltageByte v372 = VoltageByte::fromVoltage(3.72f);
const VoltageByte v381 = VoltageByte::fromVoltage(3.81f);
const VoltageByte v392 = VoltageByte::fromVoltage(3.92f);
const VoltageByte v400 = VoltageByte::fromVoltage(4.00f);
const VoltageByte v420 = VoltageByte::fromVoltage(4.20f);

VoltageByte VoltageByte::getMaxVoltage() {
    return v420;
}

VoltageByte VoltageByte::getMinVoltage() {
    return v275;
}

/**
 * @brief Calculate State of Charge (SoC2) from voltage byte
 * 
 * This implements piecewise linear interpolation.
 * It uses the input voltage to estimate battery state of charge.
 * Formulas have been algebraically simplified for efficiency.
 * 
 * @return State of Charge as percentage (0.0 to 100.0)
 */
float VoltageByte::toSoC2() const {
    
    float soc;
    
    if (*this < v275) {
        soc = 0.0f;
    } 
    else if (*this < v300) {
        // Simplified: 0.04082 * value_ + (0.04082 * 210) - 11.2255
        soc = 0.04082f * value_ - 2.6533f;
    } 
    else if (*this < v347) {
        // Simplified: 0.33497 * value_ + (0.33497 * 210) - 99.471
        soc = 0.33497f * value_ - 29.1273f;
    } 
    else if (*this < v360) {
        // Simplified: 1.32143 * value_ + (1.32143 * 210) - 441.573
        soc = 1.32143f * value_ - 164.0727f;
    } 
    else if (*this < v372) {
        // Simplified: 1.83199 * value_ + (1.83199 * 210) - 625.684
        soc = 1.83199f * value_ - 241.0661f;
    } 
    else if (*this < v381) {
        // Simplified: 0.89213 * value_ + (0.89213 * 210) - 275.962
        soc = 0.89213f * value_ - 88.6147f;
    } 
    else if (*this < v392) {
        // Simplified: 1.31098 * value_ + (1.31098 * 210) - 435.5
        soc = 1.31098f * value_ - 160.1942f;
    }
    else if (*this < v400) {
        // Simplified: 1.00031 * value_ + (1.00031 * 210) - 313.686
        soc = 1.00031f * value_ - 103.6209f;
    } 
    else if (*this < v420) {
        // Simplified: 1.35913 * value_ + (1.35913 * 210) - 457.106
        soc = 1.35913f * value_ - 171.6887f;
    } 
    else {
        soc = 113.727f; // Maximum value (slightly above 100%)
    }
    
    // Clamp to [0, 100] range
    if (soc < 0.0f) soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    
    return soc;
}
