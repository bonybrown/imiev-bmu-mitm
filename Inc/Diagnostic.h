#pragma once

#include <cstdint>
#include "CanMessage374Override.h"
#include "CanMessage374.h"
#include "can_types.h"
#include "CanQueue.h"

/**
 * @brief Diagnostic functions for battery management system
 * 
 * This class provides diagnostic and override capabilities for the battery
 * management system, including temperature override functionality.
 */
class Diagnostic {
public:
    constexpr static uint16_t COMMAND_MESSAGE_ID = 0x721; ///< Diagnostic command message ID
    constexpr static uint16_t HEARTBEAT_MESSAGE_ID = 0x720; ///< Diagnostic heartbeat message ID
    static constexpr uint8_t FUNCTION_SET_TEMPERATURE_OVERRIDE = 0x01; // processSetTemperatureOverride function code

    Diagnostic();

    /**
     * @brief Process a diagnostic command from a CAN frame
     * @param frame CAN frame containing diagnostic command
     * 
     * The frame's data[0] byte determines the function:
     * - Function 1: Set cell temperature override for 10 seconds
     *   - data[1]: Min temperature in usual CAN format (single byte, 50 degree offset)
     *   - data[2]: Max temperature in usual CAN format (single byte, 50 degree offset)
     */
    void processCommand(const CAN_FRAME& frame);

    /**
     * @brief Apply active overrides to a CAN message 0x374
     * @param msg Reference to CanMessage374 to apply overrides to
     * @return true if any override was applied, false otherwise
     */
    bool applyOverrides(CanMessage374& msg);

    /**
     * @brief Send a diagnostic CAN message (formerly heartbeat)
     * @param txQueue Pointer to the TxQueue for sending messages
     * @param seconds Uptime in seconds
     */
    void sendDiagnosticData(CanQueue<QUEUE_CAPACITY>* txQueue, uint32_t seconds);
private:
    CanMessage374Override m_canMessage374Override;

    /**
     * @brief Process function 1: Set temperature override
     * @param frame CAN frame containing temperature override parameters
     */
    void processSetTemperatureOverride(const CAN_FRAME& frame);
};
