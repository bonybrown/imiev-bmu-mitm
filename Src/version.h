#pragma once
#include <cstdint>
/**
 * @file version.h
 * @brief Project version information
 *
 * This file contains the version information for the project.
 */
namespace ProjectVersion {
    const uint8_t MAJOR = 2; ///< Major version number
    const uint8_t MINOR = 0; ///< Minor version number
}

/*
Version History:
- Version 1.0: Initial release, C++ version of https://github.com/iso14000/MIevM/tree/main, added unit tests and improved type safety.
There is no sleep mode implemented, nor any regenerative braking control.
Added heartbeat message with uptime.

- Version 1.1: Ensure SoC2 recalibration occurs every 60 seconds, not just once after rest.
Do not send CAN 0x374 response until battery model is initialized.

- Version 2.0: Added diagnostic command processing for temperature override.
Send message 0x721 [01 minTemp maxTemp] to override cell temperatures in 0x374 for 10 seconds.
Implemented safety checks to prevent unrealistic temperature changes.
Original cell min/max temperatures are sent in the 0x720 periodic message bytes 2 and 3.
All temperature values on the CAN bus are one byte, temperature - 50 (ie, 0x01 is -49 C, 0x32 is 0 C)
*/