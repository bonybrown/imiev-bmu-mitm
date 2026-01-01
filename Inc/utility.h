/**
 * @file utility.h
 * @brief Utility functions for common operations
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate time difference between two tick values, handling overflow
 * @param currentTick Current tick value
 * @param lastTick Previous tick value
 * @return Time difference in ticks, accounting for potential overflow
 * 
 * This function correctly handles the case where a 32-bit timer wraps around
 * from UINT32_MAX to 0. It calculates the elapsed time even across the overflow
 * boundary.
 */
uint32_t CalculateTickDifference(uint32_t currentTick, uint32_t lastTick);

#ifdef __cplusplus
}
#endif

#endif // UTILITY_H
