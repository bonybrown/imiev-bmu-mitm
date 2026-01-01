/**
 * @file utility.c
 * @brief Utility functions for common operations
 */

#include "utility.h"
#include <stdint.h>

/**
 * @brief Calculate time difference between two tick values, handling overflow
 * @param currentTick Current tick value
 * @param lastTick Previous tick value
 * @return Time difference in ticks, accounting for potential overflow
 * 
 * This function correctly handles the case where a 32-bit timer wraps around
 * from UINT32_MAX to 0. It calculates the elapsed time even across the overflow
 * boundary.
 * 
 * Examples:
 * - CalculateTickDifference(1005, 1000) = 5 (normal case)
 * - CalculateTickDifference(0, UINT32_MAX) = 1 (overflow from max to 0)
 * - CalculateTickDifference(10, UINT32_MAX - 5) = 16 (overflow case)
 */
uint32_t CalculateTickDifference(uint32_t currentTick, uint32_t lastTick)
{
    uint32_t diff = 0;
    
    if (currentTick < lastTick)
    {
        // Timer overflow: calculate time including the wrap
        diff = UINT32_MAX - lastTick + currentTick + 1;
    }
    else
    {
        // Normal case: simple subtraction
        diff = currentTick - lastTick;
    }
    
    return diff;
}
