#pragma once
#include <cstdint>
/**
 * @file version.h
 * @brief Project version information
 *
 * This file contains the version information for the project.
 */
namespace ProjectVersion {
    const uint8_t MAJOR = 1; ///< Major version number
    const uint8_t MINOR = 1; ///< Minor version number
}

/*
Version History:
- Version 1.0: Initial release, C++ version of https://github.com/iso14000/MIevM/tree/main, added unit tests and improved type safety.
There is no sleep mode implemented, nor and regenerative braking control.
Added heartbeat message with uptime.

- Version 1.1: Ensure SoC2 recalibration occurs every 60 seconds, not just once after rest.
Do not send CAN 0x374 response until battery model is initialized.


*/