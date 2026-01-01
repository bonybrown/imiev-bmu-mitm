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
